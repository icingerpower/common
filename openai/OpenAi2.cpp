#include <QBuffer>
#include <QCryptographicHash>
#include <QMimeDatabase>
#include <QMimeType>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QSet>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>
#include <QDir>

//#include "ExceptionOpenAiError.h"
#include "ExceptionOpenAiNotInitialized.h"

#include "OpenAi2.h"
#include <QCoro/QCoroFuture>

std::function<QString(const QList<QString> &gptReplies)> OpenAi2::CHOOSE_MOST_FREQUENT
= [](const QList<QString> &gptReplies) -> QString {
    if (gptReplies.isEmpty()) return QString{};

    QMap<QString, int> counts;
    for (const auto &r : gptReplies)
    {
        counts[r]++;
    }
    
    QString best;
    int maxCount = -1;
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it)
    {
        if (it.value() > maxCount)
        {
             maxCount = it.value();
             best = it.key();
        }
    }
    return best;
};

std::function<QString(const QList<QString> &gptReplies)> OpenAi2::CHOOSE_ALL_SAME_OR_EMPTY
= [](const QList<QString> &gptReplies) -> QString {
    if (gptReplies.size() == 0)
    {
        return QString{};
    }
    QSet<QString> replies;
    for (const auto &gptReply : gptReplies)
    {
        replies.insert(gptReply);
        if (replies.size() > 1)
        {
            return QString{};
        }
    }
    return gptReplies[0];
};

const QString OpenAi2::KEY_OPEN_AI_API_KEY = "open-ai-api-key";
const QUrl OpenAi2::RESPONSES_URL("https://api.openai.com/v1/responses");

OpenAi2 *OpenAi2::instance()
{
    static OpenAi2 instance;
    return &instance;
}

OpenAi2::OpenAi2(QObject *parent)
    : QObject(parent)
{
    m_maxQueriesSameTime = 5;
    m_maxQueriesImageSameTime = 1;
    m_inFlightText = 0;
    m_inFlightImage = 0;
    m_pumping = false;
    m_blockedUntilMs = 0;
    m_consecutiveHardFailures = 0;
    m_lastImageQueryStartTimeMs = 0;
    m_pumpTimer = nullptr;

#ifdef OPENAI2_UNIT_TESTS
    m_transport = [this](const QString &model, const QString &prompt, const QList<QString> &imagePaths, std::function<void(QString)> onOk, std::function<void(TransportError)> onErr)
    {
        this->_callResponses_Real(model, prompt, imagePaths, onOk, onErr);
    };
#endif

    m_pumpTimer = new QTimer(this);
    m_pumpTimer->setSingleShot(true);
    connect(m_pumpTimer, &QTimer::timeout, this, &OpenAi2::_pumpLoop);
}

void OpenAi2::init(const QString &apiKey
                   , int maxQueriesSameTime
                   , int maxQueriesImageSameTime
                   , int timeoutMsBetweenImageQueries) {
    m_openAiKey = apiKey;
    m_initialized = true;
    m_maxQueriesSameTime = maxQueriesSameTime;
    m_maxQueriesImageSameTime = maxQueriesImageSameTime;
    m_timeoutMsBetweenImageQueries = timeoutMsBetweenImageQueries;
}

#ifdef OPENAI2_UNIT_TESTS
void OpenAi2::setTransportForTests(std::function<void(const QString&, const QString&, const QList<QString>&, std::function<void(QString)>, std::function<void(TransportError)>)> transport)
{
    m_transport = transport;
}

void OpenAi2::resetForTests()
{
    m_initialized = false;
    m_openAiKey = "";
    m_pendingText.clear();
    m_pendingImage.clear();
    m_inFlightText = 0;
    m_inFlightImage = 0;
    m_pumping = false;
    m_blockedUntilMs = 0;
    m_consecutiveHardFailures = 0;
    m_transport = nullptr;
    m_lastImageQueryStartTimeMs = 0;
    if (m_pumpTimer) m_pumpTimer->stop();
    m_timeProvider = nullptr; // Reset time provider
    
    m_transport = [this](const QString &m, const QString &p, const QList<QString> &i, std::function<void(QString)> ok, std::function<void(TransportError)> err) {
         this->_callResponses_Real(m, p, i, ok, err);
    };
}

void OpenAi2::setTimeProviderForTests(std::function<qint64()> provider)
{
    m_timeProvider = provider;
}


qint64 OpenAi2::_now() const
{
#ifdef OPENAI2_UNIT_TESTS
    if (m_timeProvider) return m_timeProvider();
#endif
    return QDateTime::currentMSecsSinceEpoch();
}

OpenAi2::DebugState OpenAi2::getDebugStateForTests() const
{
    DebugState s;
    s.initialized = m_initialized;
    s.maxQueriesSameTime = m_maxQueriesSameTime;
    s.inFlightText = m_inFlightText;
    s.inFlightImage = m_inFlightImage;
    s.pumping = m_pumping;
    s.blockedUntilMs = m_blockedUntilMs;
    s.consecutiveHardFailures = m_consecutiveHardFailures;
    s.pendingTextSize = m_pendingText.size();
    s.pendingImageSize = m_pendingImage.size();
    return s;
}
#endif

QString OpenAi2::Step::getGptModel(int attempt) const
{
    if (this->maxRetries <= 0)
    {
        return this->gptModel;
    }

    int half = 0;
    half = this->maxRetries / 2;

    if (attempt > half)
    {
        return this->gptModelAfterHalfFailure;
    }
    return this->gptModel;
}

void OpenAi2::askGpt(const QList<QSharedPointer<Step>> &stepsInQueue, const QString &model)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    QList<QSharedPointer<Step>> steps;
    steps = stepsInQueue;

    this->_runQueue(
        steps,
        []()
        {
        },
        [](QString)
        {
        });

    Q_UNUSED(model);
}

void OpenAi2::askGptMultipleTime(
        const QList<QSharedPointer<StepMultipleAsk>> &stepsInQueue, const QString &model)
{
    this->_runMultipleTime(stepsInQueue, model, nullptr, nullptr);
}

QCoro::Task<void> OpenAi2::askGptMultipleTimeCoro(
        const QList<QSharedPointer<StepMultipleAsk>> &stepsInQueue, const QString &model)
{
    // Use QSharedPointer to share the promise state safely across lambdas
    auto promise = QSharedPointer<QFutureInterface<void>>::create();
    promise->reportStarted();
    QFuture<void> future = promise->future();

    this->_runMultipleTime(
        stepsInQueue, 
        model, 
        [promise]() {
            promise->reportFinished();
        },
        [promise](QString err) {
             qWarning() << "askGptMultipleTimeCoro failed:" << err;
             // Always finish the future to unblock the awaiter.
             promise->reportFinished();
        }
    );

    co_await future;
}

void OpenAi2::_runMultipleTime(
         const QList<QSharedPointer<StepMultipleAsk>> &stepsInQueue
         , const QString &model
         , std::function<void()> onAllSuccess
         , std::function<void(QString)> onAllFailure)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }
    
    Q_UNUSED(model); // model is usually inside step or handled by runStep logic

    if (stepsInQueue.isEmpty())
    {
        if (onAllSuccess) onAllSuccess();
        return;
    }

    QSharedPointer<int> idx = QSharedPointer<int>::create(0);
    // Trampoline for recursion
    QSharedPointer<std::function<void()>> runNext = QSharedPointer<std::function<void()>>::create();
    
    // Capture callbacks by copy
    *runNext = [this, stepsInQueue, idx, runNext, onAllSuccess, onAllFailure]()
    {
        if ((*idx) >= stepsInQueue.size())
        {
            if (onAllSuccess) onAllSuccess();
            return;
        }

        auto step = stepsInQueue[*idx];


        this->_runStepCollectN(
            step,
            step->neededReplies,
            step->chooseBest, 
            [step, idx, runNext](QString best)
            {
                if (step->apply)
                {
                    step->apply(best);
                }
                (*idx) = (*idx) + 1;
                QTimer::singleShot(0, [runNext](){ (*runNext)(); });
            },
            [idx, onAllFailure](QString err)
            {
                qWarning() << "askGptMultipleTime failed step" << *idx << ":" << err;
                // Stop execution on failure, consistent with _runQueue
                if (onAllFailure) onAllFailure(err);
            }
        );
    };

    (*runNext)();
}

void OpenAi2::askGptMultipleTimeAi(const QList<QSharedPointer<StepMultipleAskAi>> &stepsInQueue, const QString &model)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    QList<QSharedPointer<Step>> steps;
    steps.reserve(stepsInQueue.size());

    for (int i = 0; i < stepsInQueue.size(); i++)
    {
        QSharedPointer<StepMultipleAskAi> s;
        s = stepsInQueue[i];

        QSharedPointer<Step> base;
        base = qSharedPointerCast<Step>(s);

        steps.push_back(base);
    }

    this->_runQueue(
        steps,
        []()
        {
        },
        [](QString)
        {
        });

    Q_UNUSED(model);
}

void OpenAi2::askGptBatch(const QList<QSharedPointer<Step>> &stepsIndependants,
                          const QString &model,
                          const QString &progressCsvFilePath,
                          int nQueriesLeftForNonBatch)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    QList<QSharedPointer<Step>> steps;
    steps = stepsIndependants;

    this->_runBatch(
        steps,
        model,
        progressCsvFilePath,
        nQueriesLeftForNonBatch,
        []()
        {
        },
        [](QString)
        {
        });
}

void OpenAi2::askGptBatchMultipleTime(const QList<QSharedPointer<StepMultipleAsk>> &stepsIndependants,
                                      const QString &model,
                                      const QString &progressCsvFilePath,
                                      int nQueriesLeftForNonBatch)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    QList<QSharedPointer<Step>> steps;
    steps.reserve(stepsIndependants.size());

    for (int i = 0; i < stepsIndependants.size(); i++)
    {
        QSharedPointer<StepMultipleAsk> s;
        s = stepsIndependants[i];

        QSharedPointer<Step> base;
        base = qSharedPointerCast<Step>(s);

        steps.push_back(base);
    }

    this->_runBatch(
        steps,
        model,
        progressCsvFilePath,
        nQueriesLeftForNonBatch,
        []()
        {
        },
        [](QString)
        {
        });
}

void OpenAi2::askGptBatchMultipleTimeAi(const QList<QSharedPointer<StepMultipleAskAi>> &stepsIndependants,
                                        const QString &model,
                                        const QString &progressCsvFilePath,
                                        int nQueriesLeftForNonBatch)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    QList<QSharedPointer<Step>> steps;
    steps.reserve(stepsIndependants.size());

    for (int i = 0; i < stepsIndependants.size(); i++)
    {
        QSharedPointer<StepMultipleAskAi> s;
        s = stepsIndependants[i];

        QSharedPointer<Step> base;
        base = qSharedPointerCast<Step>(s);

        steps.push_back(base);
    }

    this->_runBatch(
        steps,
        model,
        progressCsvFilePath,
        nQueriesLeftForNonBatch,
        []()
        {
        },
        [](QString)
        {
        });
}

void OpenAi2::_runQueue(const QList<QSharedPointer<Step>> &steps,
                        std::function<void()> onAllSuccess,
                        std::function<void(QString)> onAllFailure)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    if (steps.isEmpty())
    {
        if (onAllSuccess)
        {
            onAllSuccess();
        }
        return;
    }

    QSharedPointer<int> idx;
    idx = QSharedPointer<int>::create(0);

    QSharedPointer<QString> firstError;
    firstError = QSharedPointer<QString>::create("");

    // Trampoline to avoid recursive lambda by reference
    QSharedPointer<std::function<void()>> runNext = QSharedPointer<std::function<void()>>::create();
    (*runNext) = [this, steps, onAllSuccess, onAllFailure, idx, firstError, runNext]()
    {
        if ((*idx) >= steps.size())
        {
            if (firstError->isEmpty())
            {
                if (onAllSuccess)
                {
                    onAllSuccess();
                }
            }
            else
            {
                if (onAllFailure)
                {
                    onAllFailure(*firstError);
                }
            }
            return;
        }

        QSharedPointer<Step> step;
        step = steps[(*idx)];

        this->_runStepWithRetries(
            step,
            [this, idx, runNext](QString raw)
            {
                Q_UNUSED(raw);
                (*idx) = (*idx) + 1;
                (*runNext)();
            },
            [this, idx, firstError, onAllFailure, runNext](QString err)
            {
                if (firstError->isEmpty())
                {
                    (*firstError) = err;
                }

                (*idx) = (*idx) + 1;
                (*runNext)();

                Q_UNUSED(onAllFailure);
            });
    };

    (*runNext)();
}

void OpenAi2::_runStepWithRetries(const QSharedPointer<Step> &step,
                                  std::function<void(QString raw)> onStepSuccess,
                                  std::function<void(QString err)> onStepFailure)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }


    if (step.isNull())
    {
        if (onStepFailure)
        {
            onStepFailure("Step is null");
        }
        return;
    }

    QString cached;
    cached = "";

    QString *cachedPtr = nullptr;
    cachedPtr = &cached;

    if (this->_tryLoadCache(*step.data(), cachedPtr))
    {
        // Force async to keep behavior consistent and prevent caller (e.g. tests) 
        // from missing synchronous callbacks before event loop starts.
        QTimer::singleShot(0, this, [step, cached, onStepSuccess]() {
            if (step->apply)
            {
                step->apply(cached);
            }
            if (onStepSuccess)
            {
                onStepSuccess(cached);
            }
        });
        return;
    }

    QSharedPointer<int> attempt;
    attempt = QSharedPointer<int>::create(0);

    QSharedPointer<QString> lastWhy;
    lastWhy = QSharedPointer<QString>::create("");

    QSharedPointer<std::function<void()>> doAttempt = QSharedPointer<std::function<void()>>::create();
    (*doAttempt) = [this, step, attempt, lastWhy, onStepSuccess, onStepFailure, doAttempt]()
    {
        int a = 0;
        a = (*attempt);

        QString prompt;
        prompt = "";

        if (step->getPrompt)
        {
            prompt = step->getPrompt(a);
        }

        QString model;
        model = this->_selectModelForAttempt(*step.data(), a);

        this->_callResponses(
            model,
            prompt,
            step->imagePaths,
            [this, step, attempt, lastWhy, onStepSuccess, onStepFailure, doAttempt](QString raw)
            {
                bool ok = true;
                ok = true;

                if (step->validate)
                {
                    ok = step->validate(raw, *lastWhy);
                }

                if (!ok)
                {
                    (*lastWhy) = "validate_failed";
                    (*attempt) = (*attempt) + 1;

                    int maxRetries = 0;
                    maxRetries = step->maxRetries;

                    if (maxRetries <= 0)
                    {
                        maxRetries = 1;
                    }

                    if ((*attempt) >= maxRetries)
                    {
                        // FAIL: retries exhausted. 
                        // CRITICAL FIX: Do NOT apply() or cache() invalid/failed result.
                        if (onStepFailure)
                        {
                            onStepFailure(*lastWhy);
                        }
                        return;
                    }

                    QTimer::singleShot(0, this, [this, doAttempt]()
                    {
                        (*doAttempt)();
                    });
                    return;
                }

                if (step->apply)
                {
                    step->apply(raw);
                }
                this->_storeCache(*step.data(), raw);
                if (onStepSuccess)
                {
                    onStepSuccess(raw);
                }
            },
            [this, step, attempt, lastWhy, onStepFailure, doAttempt](TransportError err)
            {
                (*lastWhy) = err.message;
                (*attempt) = (*attempt) + 1;

                int maxRetries = 0;
                maxRetries = step->maxRetries;

                if (maxRetries <= 0)
                {
                    maxRetries = 1;
                }
                
                // FATAL: stop immediately
                if (err.isFatal)
                {
                     (*attempt) = maxRetries + 1; // Force stop
                }

                if ((*attempt) >= maxRetries)
                {
                    if (onStepFailure)
                    {
                        onStepFailure(err.message);
                    }
                    return;
                }

                QTimer::singleShot(0, this, [this, doAttempt]()
                {
                    (*doAttempt)();
                });
            });
    };

    (*doAttempt)();
}

void OpenAi2::_callResponses(const QString &model,
                             const QString &prompt,
                             const QList<QString> &imagePaths,
                             std::function<void(QString raw)> onOk,
                             std::function<void(TransportError err)> onErr)
{
    bool isImage = !imagePaths.isEmpty();

    // Wrap validation/state tracking logic
    auto wrappedOk = [this, onOk](QString raw) {
        this->_onInternalCallSuccess();
        if (onOk) onOk(raw);
    };
    auto wrappedErr = [this, onErr](TransportError e) {
        this->_onInternalCallError(e);
        if (onErr) onErr(e);
    };

    // SCHEDULE HERE
    // This ensures both Real and Fake transports (used in tests) respect the scheduler/throttling.
    _schedule([this, model, prompt, imagePaths, wrappedOk, wrappedErr](std::function<void()> done) {
        
        // Wrap callbacks to trigger 'done' when finished
        auto doneOk = [wrappedOk, done](QString raw) {
            wrappedOk(raw);
            done(); 
        };
        auto doneErr = [wrappedErr, done](TransportError e) {
            wrappedErr(e);
            done();
        };

        try {
#ifdef OPENAI2_UNIT_TESTS
            if (m_transport)
            {
                m_transport(model, prompt, imagePaths, doneOk, doneErr);
                return;
            }
#endif
            this->_callResponses_Real(model, prompt, imagePaths, doneOk, doneErr);

        } catch (const std::exception &e) {
            TransportError err{}; 
            err.type = TransportError::Unknown; 
            err.message = QString("transport_exception:%1").arg(e.what());
            err.isFatal = false;
            doneErr(err);
        } catch (...) {
            TransportError err{}; 
            err.type = TransportError::Unknown; 
            err.message = "transport_exception:unknown";
            err.isFatal = false;
            doneErr(err);
        }
    }, isImage);
}

void OpenAi2::_onInternalCallSuccess()
{
    m_consecutiveHardFailures = 0;
}

void OpenAi2::_onInternalCallError(const TransportError &err)
{
    if (err.isFatal)
    {
        m_consecutiveHardFailures++;
        if (m_consecutiveHardFailures >= 5) 
        {
             // Circuit breaker logic placeholder
        }
    }
}


void OpenAi2::_callResponses_Real(const QString &model,
                             const QString &prompt,
                             const QList<QString> &imagePaths,
                             std::function<void(QString raw)> onOk,
                             std::function<void(TransportError err)> onErr)
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }

    if (m_openAiKey.trimmed().isEmpty())
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }



    QJsonObject root;
    root.insert("model", model);

    QJsonArray input;
    QJsonObject userMsg;
    userMsg.insert("role", "user");

    QJsonArray content;

    QJsonObject textObj;
    textObj.insert("type", "input_text");
    textObj.insert("text", prompt);
    content.append(textObj);

    for (int i = 0; i < imagePaths.size(); i++)
    {
        QString path;
        path = imagePaths[i];

        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
        {
            if (onErr)
            {
                TransportError e{};
                e.type = TransportError::Unknown;
                e.message = QString("cannot_open_image:%1").arg(path);
                e.isRetryable = false;
                onErr(e);
            }
            return;
        }

        QByteArray bytes;
        bytes = f.readAll();
        f.close();

        QMimeDatabase db;
        QMimeType mt;
        mt = db.mimeTypeForFile(path);

        QString mime;
        mime = mt.name();

        if (mime.trimmed().isEmpty())
        {
            mime = "application/octet-stream";
        }

        QString dataUrl;
        dataUrl = "data:" + mime + ";base64," + QString::fromLatin1(bytes.toBase64());

        QJsonObject imgObj;
        imgObj.insert("type", "input_image");
        imgObj.insert("image_url", dataUrl);
        content.append(imgObj);
    }

    userMsg.insert("content", content);
    input.append(userMsg);

    root.insert("input", input);

    QJsonDocument doc(root);
    QByteArray payload;
    payload = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest req(RESPONSES_URL);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString auth;
    auth = "Bearer " + m_openAiKey;
    req.setRawHeader("Authorization", auth.toUtf8());
    req.setRawHeader("Accept", "application/json");
    req.setRawHeader("User-Agent", "OpenAi2-QtTest/1.0");

    // Execute immediately (scheduler is handled by caller _callResponses)
    QNetworkReply *reply = nullptr;
    reply = m_networkAccessManager.post(req, payload);

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, onOk, onErr]()
    {
        // Helper to cleanup reply. 
        // Note: done() is handled by the wrapper passed as onOk/onErr.
        auto finish = [reply]() {
            reply->deleteLater();
        };

        QByteArray body;
        body = reply->readAll();

        int httpStatus = 0;
        httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QNetworkReply::NetworkError netErr = reply->error();
        const QVariant wwwAuth = reply->rawHeader("www-authenticate");
        qWarning() << "Body (first 1024):" << QString::fromUtf8(body.left(1024));

        if (netErr != QNetworkReply::NoError)
        {
            TransportError err{};
            err.type = TransportError::NetworkError;
            err.httpStatus = httpStatus;
            err.message = QString("network_error:%1:http:%2:%3")
                      .arg(static_cast<int>(netErr))
                      .arg(httpStatus)
                      .arg(QString::fromUtf8(body.left(512)));
            err.isRetryable = true;
            err.isFatal = false;

            if (onErr)
            {
                onErr(err);
            }
            finish();
            return;
        }

        if (httpStatus >= 400)
        {
            TransportError err{};
            err.type = TransportError::HttpError;
            err.httpStatus = httpStatus;
            
            bool isFatal = (httpStatus == 400 || httpStatus == 401 || httpStatus == 403);
            err.isFatal = isFatal;
            err.isRetryable = !isFatal;

            if (isFatal) {
                 err.message = QString("fatal:http_error:%1:%2")
                      .arg(httpStatus)
                      .arg(QString::fromUtf8(body.left(1024)));
            } else {
                 err.message = QString("http_error:%1:%2")
                      .arg(httpStatus)
                      .arg(QString::fromUtf8(body.left(1024)));
            }

            if (httpStatus == 429)
            {
                 m_blockedUntilMs = _now() + 2 * 60 * 1000;
            }

            if (onErr)
            {
                onErr(err);
            }
            finish();
            return;
        }

        // Success
        // m_consecutiveHardFailures = 0;

        QString raw;
        QString text;

        QJsonParseError pe;
        QJsonDocument doc = QJsonDocument::fromJson(body, &pe);
        if (pe.error == QJsonParseError::NoError && doc.isObject())
        {
            QJsonObject root = doc.object();
            QJsonArray output = root.value("output").toArray();
            for (const QJsonValue& v : output)
            {
                QJsonObject o = v.toObject();
                if (o.value("type").toString() != "message")
                    continue;

                QJsonArray content = o.value("content").toArray();
                for (const QJsonValue& cv : content)
                {
                    QJsonObject c = cv.toObject();
                    if (c.value("type").toString() == "output_text")
                    {
                        text = c.value("text").toString();
                        break;
                    }
                }
                if (!text.isEmpty()) break;
            }
        }

        if (text.isEmpty())
        {
            if (onErr) {
                TransportError te{};
                te.type = TransportError::ParseError;
                te.message = "parse_error:no_output_text";
                te.isRetryable = true; 
                te.isFatal = false;
                onErr(te);
            }
            finish();
            return;
        }

        if (onOk)
        {
            onOk(text);
        }
        finish();

    });
}

bool OpenAi2::_tryLoadCache(const Step &step, QString *rawOut) const
{
    if (rawOut == nullptr)
    {
        return false;
    }

    if (step.cachingKey.trimmed().isEmpty())
    {
        return false;
    }

    QString dirPath;
    dirPath = QDir::tempPath() + "/openai2_cache";

    QDir d(dirPath);
    if (!d.exists())
    {
        return false;
    }

    QByteArray keyBytes;
    keyBytes = step.cachingKey.toUtf8();

    QByteArray hash;
    hash = QCryptographicHash::hash(keyBytes, QCryptographicHash::Sha256);

    QString filePath;
    filePath = dirPath + "/" + QString::fromLatin1(hash.toHex()) + ".json";

    QFile f(filePath);
    if (!f.exists())
    {
        return false;
    }

    if (!f.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray bytes;
    bytes = f.readAll();
    f.close();

    (*rawOut) = QString::fromUtf8(bytes);
    return true;
}

void OpenAi2::_storeCache(const Step &step, const QString &raw) const
{
    if (step.cachingKey.trimmed().isEmpty())
    {
        return;
    }

    QString dirPath;
    dirPath = QDir::tempPath() + "/openai2_cache";

    QDir d;
    if (!d.exists(dirPath))
    {
        d.mkpath(dirPath);
    }

    QByteArray keyBytes;
    keyBytes = step.cachingKey.toUtf8();

    QByteArray hash;
    hash = QCryptographicHash::hash(keyBytes, QCryptographicHash::Sha256);

    QString filePath;
    filePath = dirPath + "/" + QString::fromLatin1(hash.toHex()) + ".json";

    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return;
    }

    QByteArray bytes;
    bytes = raw.toUtf8();

    f.write(bytes);
    f.close();
}

QString OpenAi2::_selectModelForAttempt(const Step &step, int attempt) const
{
    return step.getGptModel(attempt);
}

void OpenAi2::_schedule(std::function<void(std::function<void()>)> startRequest, bool isImage)
{
    if (startRequest)
    {
        if (isImage) m_pendingImage.enqueue(startRequest);
        else m_pendingText.enqueue(startRequest);
    }
    _requestPump(0);
}

void OpenAi2::_requestPump(int delayMs)
{
    if (delayMs < 0) delayMs = 0;

    if (m_pumpTimer->isActive())
    {
        int remaining = m_pumpTimer->remainingTime();
        if (remaining <= delayMs) return; // Existing timer is sooner/same
    }
    
    m_pumpTimer->start(delayMs);
}

void OpenAi2::_pumpLoop()
{
    if (m_pumping) return;
    m_pumping = true;

    qint64 now = _now();
    
    // Debug logging for T2
    // qWarning() << "_pumpLoop start. Now:" << now << "BlockedUntil:" << m_blockedUntilMs << "InFlightTxt:" << m_inFlightText << "InFlightImg:" << m_inFlightImage;

    // 1. Blocked Until Check
    if (m_blockedUntilMs > now)
    {
        _requestPump(m_blockedUntilMs - now + 100);
        m_pumping = false;
        return;
    }

    while(true)
    {
        bool didWork = false;
        now = _now(); // Update time

        // --- IMAGE ---
        if (!m_pendingImage.isEmpty())
        {
            if (m_inFlightImage < m_maxQueriesImageSameTime)
            {
                qint64 elapsed = now - m_lastImageQueryStartTimeMs;
                if (m_timeoutMsBetweenImageQueries > 0 && elapsed < m_timeoutMsBetweenImageQueries)
                {
                   // Throttled. Schedule wake up but DO NOT STOP checking text.
                   // qWarning() << "Image throttled. Elapsed:" << elapsed << "Timeout:" << m_timeoutMsBetweenImageQueries;
                   _requestPump(m_timeoutMsBetweenImageQueries - elapsed + 10);
                }
                else
                {
                    // Run Image
                    auto job = m_pendingImage.dequeue();
                    m_inFlightImage++;
                    m_lastImageQueryStartTimeMs = now;
                    didWork = true;

                    auto guard = QSharedPointer<std::atomic_bool>::create(false);
                    auto doneCallback = [this, guard]() {
                        bool expected = false;
                        if (guard->compare_exchange_strong(expected, true)) {
                            m_inFlightImage--;
                            if (m_inFlightImage < 0) m_inFlightImage = 0;
                            _requestPump(0);
                        }
                    };

                    try {
                        job(doneCallback);
                    } catch (...) {
                        doneCallback();
                    }
                }
            }
        }

        // --- TEXT ---
        if (!m_pendingText.isEmpty())
        {
             // qWarning() << "Checking Text Queue. Count:" << m_pendingText.size() << "InFlight:" << m_inFlightText << "Max:" << m_maxQueriesSameTime;
             if (m_inFlightText < m_maxQueriesSameTime)
             {
                 auto job = m_pendingText.dequeue();
                 // qWarning() << "Dequeued Text Job. Pending size now:" << m_pendingText.size();
                 m_inFlightText++;
                 didWork = true;

                 auto guard = QSharedPointer<std::atomic_bool>::create(false);
                 auto doneCallback = [this, guard]() {
                     bool expected = false;
                     if (guard->compare_exchange_strong(expected, true)) {
                         m_inFlightText--;
                         if (m_inFlightText < 0) m_inFlightText = 0;
                         _requestPump(0);
                     }
                 };

                 try {
                     job(doneCallback);
                 } catch (...) {
                     doneCallback();
                 }
             }
        }

        if (!didWork) break;
    }

    m_pumping = false;
}

void OpenAi2::_runStepCollectN(const QSharedPointer<Step> &step,
                               int neededReplies,
                               std::function<QString(const QList<QString> &valids)> chooseBest,
                               std::function<void(QString best)> onBest,
                               std::function<void(QString err)> onFail)
{
    if (neededReplies <= 0)
    {
        if (onFail)
        {
            onFail("neededReplies<=0");
        }
        return;
    }

    QSharedPointer<QList<QString>> valids;
    valids = QSharedPointer<QList<QString>>::create();

    QSharedPointer<int> attempts;
    attempts = QSharedPointer<int>::create(0);

    QSharedPointer<std::function<void()>> one = QSharedPointer<std::function<void()>>::create();
    (*one) = [this, step, neededReplies, chooseBest, onBest, onFail, valids, attempts, one]()
    {
        // Use a proxy step to prevent _runStepWithRetries from calling step->apply() on each attempt.
        // We only want to apply the FINAL aggregated result.
        // Copying *step (which references the functions/data) is cheap and safe (slicing is fine as usage is generic).
        auto proxyStep = QSharedPointer<Step>::create(*step);
        proxyStep->apply = nullptr;

        this->_runStepWithRetries(
            proxyStep,
            [this, neededReplies, chooseBest, onBest, onFail, valids, attempts, one](QString raw)
            {
                valids->push_back(raw);

                if (valids->size() >= neededReplies)
                {
                    QString best;
                    best = "";

                    if (chooseBest)
                    {
                        best = chooseBest(*valids.data());
                    }
                    else
                    {
                        best = valids->first();
                    }

                    if (onBest)
                    {
                        onBest(best);
                    }
                    return;
                }



                (*attempts) = (*attempts) + 1;
                QTimer::singleShot(0, this, [this, one]()
                {
                    (*one)();
                });
            },
            [this, onFail](QString err)
            {
                if (onFail)
                {
                    onFail(err);
                }
            });
    };
    (*one)();

}

void OpenAi2::_runStepCollectNThenAskBestAI(const QSharedPointer<Step> &step,
                                            int neededReplies,
                                            std::function<QString(int attempt, const QList<QString> &valids)> getPromptGetBestReply,
                                            std::function<bool(const QString &)> validateBest,
                                            std::function<void(QString best)> onBest,
                                            std::function<void(QString err)> onFail)
{
    std::function<QString(const QList<QString> &)> chooseBest;
    chooseBest = [this, step, getPromptGetBestReply, validateBest, onFail](const QList<QString> &valids) -> QString
    {
        QSharedPointer<int> attempt;
        attempt = QSharedPointer<int>::create(0);

        QString prompt;
        prompt = "";

        if (getPromptGetBestReply)
        {
            prompt = getPromptGetBestReply((*attempt), valids);
        }

        QEventLoop loop;
        QString bestRaw;
        bestRaw = "";
        QString err;
        err = "";
        bool ok = false;
        ok = false;

        this->_callResponses(
            step->gptModel,
            prompt,
            QList<QString>(),
            [&bestRaw, &ok, &loop](QString raw)
            {
                bestRaw = raw;
                ok = true;
                loop.quit();
            },
            [&err, &ok, &loop](TransportError e)
            {
                err = e.message;
                ok = false;
                loop.quit();
            });

        loop.exec();

        if (!ok)
        {
            if (onFail)
            {
                onFail(err);
            }
            return "";
        }

        if (validateBest)
        {
            if (!validateBest(bestRaw))
            {
                if (onFail)
                {
                    onFail("validate_best_failed");
                }
                return "";
            }
        }

        return bestRaw;
    };

    this->_runStepCollectN(
        step,
        neededReplies,
        chooseBest,
        onBest,
        onFail);
}

void OpenAi2::_runBatch(const QList<QSharedPointer<Step>> &steps,
                        const QString &model,
                        const QString &progressCsv,
                        int nFallbackNonBatch,
                        std::function<void()> onAllSuccess,
                        std::function<void(QString)> onAllFailure)
{
    // This implementation is a "resumable batch runner" using a progress CSV.
    // It does NOT call OpenAI /v1/batches to avoid coupling to that API here.
    // If you want real server-side batching, implement it behind these same hooks.

    QSet<QString> alreadyDone;
    alreadyDone = QSet<QString>();

    QFile in(progressCsv);
    if (in.exists())
    {
        if (in.open(QIODevice::ReadOnly))
        {
            QTextStream ts(&in);
            while (!ts.atEnd())
            {
                QString line;
                line = ts.readLine();

                if (line.trimmed().isEmpty())
                {
                    continue;
                }

                QStringList parts;
                parts = line.split(';');

                if (parts.size() >= 1)
                {
                    QString id;
                    id = parts[0].trimmed();
                    if (!id.isEmpty())
                    {
                        alreadyDone.insert(id);
                    }
                }
            }
            in.close();
        }
    }

    QList<QSharedPointer<Step>> todo;
    todo.reserve(steps.size());

    for (int i = 0; i < steps.size(); i++)
    {
        QSharedPointer<Step> s;
        s = steps[i];

        if (s.isNull())
        {
            continue;
        }

        if (alreadyDone.contains(s->id))
        {
            continue;
        }

        todo.push_back(s);
    }

    if (todo.isEmpty())
    {
        if (onAllSuccess)
        {
            onAllSuccess();
        }
        return;
    }

    QFile out(progressCsv);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        if (onAllFailure)
        {
            onAllFailure("cannot_open_progress_csv");
        }
        return;
    }

    QTextStream ts(&out);

    QSharedPointer<int> idx;
    idx = QSharedPointer<int>::create(0);

    QSharedPointer<QString> firstErr;
    firstErr = QSharedPointer<QString>::create("");

    std::function<void()> runOne;
    runOne = [this, todo, model, nFallbackNonBatch, &ts, &out, idx, firstErr, onAllSuccess, onAllFailure, &runOne]()
    {
        if ((*idx) >= todo.size())
        {
            out.flush();
            out.close();

            if (firstErr->isEmpty())
            {
                if (onAllSuccess)
                {
                    onAllSuccess();
                }
            }
            else
            {
                if (onAllFailure)
                {
                    onAllFailure(*firstErr);
                }
            }
            return;
        }

        QSharedPointer<Step> s;
        s = todo[(*idx)];

        // If we're low on budget, force non-batch behavior: still just normal calls here,
        // but you can switch to direct calls instead of more complex re-tries.
        int budget = 0;
        budget = nFallbackNonBatch;

        Q_UNUSED(budget);

        this->_runStepWithRetries(
            s,
            [this, s, &ts, idx, &runOne](QString raw)
            {
                QString safeRaw;
                safeRaw = raw;
                safeRaw.replace("\n", "\\n");
                safeRaw.replace("\r", "\\r");

                ts << s->id << ";" << safeRaw.left(8000) << "\n";
                ts.flush();

                (*idx) = (*idx) + 1;
                runOne();
            },
            [this, idx, firstErr, &runOne](QString err)
            {
                if (firstErr->isEmpty())
                {
                    (*firstErr) = err;
                }
                (*idx) = (*idx) + 1;
                runOne();
            });
    };

    runOne();

    Q_UNUSED(model);
}

QByteArray OpenAi2::_buildBatchJsonl(const QList<QSharedPointer<Step>> &steps, const QString &model) const
{
    // Placeholder jsonl generator: each line = one Responses request payload
    QByteArray out;
    out = QByteArray();

    for (int i = 0; i < steps.size(); i++)
    {
        QSharedPointer<Step> s;
        s = steps[i];

        if (s.isNull())
        {
            continue;
        }

        QString prompt;
        prompt = "";

        if (s->getPrompt)
        {
            prompt = s->getPrompt(0);
        }

        QJsonObject root;
        root.insert("custom_id", s->id);
        root.insert("method", "POST");
        root.insert("url", "/v1/responses");

        QJsonObject body;
        body.insert("model", model);

        QJsonArray input;
        QJsonObject userMsg;
        userMsg.insert("role", "user");

        QJsonArray content;

        QJsonObject textObj;
        textObj.insert("type", "input_text");
        textObj.insert("text", prompt);
        content.append(textObj);

        userMsg.insert("content", content);
        input.append(userMsg);

        body.insert("input", input);
        root.insert("body", body);

        QJsonDocument doc(root);
        QByteArray line;
        line = doc.toJson(QJsonDocument::Compact);

        out.append(line);
        out.append("\n");
    }

    return out;
}

QHash<QString, QString> OpenAi2::_parseBatchOutput(const QByteArray &jsonlOut) const
{
    // Expected format: one JSON object per line with at least { "custom_id": "...", "response": ... }
    QHash<QString, QString> map;
    map = QHash<QString, QString>();

    QList<QByteArray> lines;
    lines = jsonlOut.split('\n');

    for (int i = 0; i < lines.size(); i++)
    {
        QByteArray line;
        line = lines[i].trimmed();

        if (line.isEmpty())
        {
            continue;
        }

        QJsonParseError pe;
        pe.error = QJsonParseError::NoError;

        QJsonDocument doc;
        doc = QJsonDocument::fromJson(line, &pe);

        if (pe.error != QJsonParseError::NoError)
        {
            continue;
        }

        if (!doc.isObject())
        {
            continue;
        }

        QJsonObject o;
        o = doc.object();

        QString id;
        id = o.value("custom_id").toString();

        if (id.isEmpty())
        {
            continue;
        }

        map.insert(id, QString::fromUtf8(line));
    }

    return map;
}
