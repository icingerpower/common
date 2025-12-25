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
#include <QDir>

//#include "ExceptionOpenAiError.h"
#include "ExceptionOpenAiNotInitialized.h"

#include "OpenAi2.h"

std::function<QString(const QList<QString> &gptReplies)> CHOOSE_MOST_FREQUENT
= [](const QList<QString> &gptReplies) -> QString {
    QHash<QString, int> reply_count;
    for (const auto &gptReply : gptReplies)
    {
        reply_count[gptReply] = reply_count[gptReply] + 1;
    }
    QMap<int, QString> count_replies;
    for (auto it = reply_count.constBegin();
         it != reply_count.constEnd(); ++it)
    {
        count_replies[it.value()] = it.key();
    }
    return count_replies.last();
};

std::function<QString(const QList<QString> &gptReplies)> CHOOSE_ALL_SAME_OR_EMPTY
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

    m_transport = [this](const QString &model, const QString &prompt, const QList<QString> &imagePaths, std::function<void(QString)> onOk, std::function<void(QString)> onErr)
    {
        this->_callResponses_Real(model, prompt, imagePaths, onOk, onErr);
    };
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

void OpenAi2::setTransportForTests(std::function<void(const QString&, const QString&, const QList<QString>&, std::function<void(QString)>, std::function<void(QString)>)> transport)
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
    
    m_transport = [this](const QString &m, const QString &p, const QList<QString> &i, std::function<void(QString)> ok, std::function<void(QString)> err) {
         this->_callResponses_Real(m, p, i, ok, err);
    };
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
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex;
        throw ex;
    }
    
    Q_UNUSED(model); // model is usually inside step or handled by runStep logic

    if (stepsInQueue.isEmpty())
    {
        return;
    }

    QSharedPointer<int> idx = QSharedPointer<int>::create(0);
    // Trampoline for recursion
    QSharedPointer<std::function<void()>> runNext = QSharedPointer<std::function<void()>>::create();

    *runNext = [this, stepsInQueue, idx, runNext]()
    {
        if ((*idx) >= stepsInQueue.size())
        {
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
            [idx](QString err)
            {
                qWarning() << "askGptMultipleTime failed step" << *idx << ":" << err;
                // Stop execution on failure, consistent with _runQueue
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
                        if (step->apply)
                        {
                            step->apply(raw);
                        }
                        this->_storeCache(*step.data(), raw);
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
            [this, step, attempt, lastWhy, onStepFailure, doAttempt](QString err)
            {
                (*lastWhy) = err;
                (*attempt) = (*attempt) + 1;

                int maxRetries = 0;
                maxRetries = step->maxRetries;

                if (maxRetries <= 0)
                {
                    maxRetries = 1;
                }
                
                // FATAL: stop immediately
                if (err.startsWith("fatal:"))
                {
                     (*attempt) = maxRetries + 1; // Force stop
                }

                if ((*attempt) >= maxRetries)
                {
                    if (onStepFailure)
                    {
                        onStepFailure(err);
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
                             std::function<void(QString err)> onErr)
{

    auto wrappedOk = [this, onOk](QString raw) {
        this->_onInternalCallSuccess();
        if (onOk) onOk(raw);
    };
    auto wrappedErr = [this, onErr](QString e) {
        this->_onInternalCallError(e);
        if (onErr) onErr(e);
    };

    if (m_transport)
    {
        m_transport(model, prompt, imagePaths, wrappedOk, wrappedErr);
    }
    else
    {
        // Even if no transport, treat as error for state tracking?
        // Probably not needed for unit tests unless we simulate it.
        // But for consistency:
        if (onErr) onErr("no_transport");
    }
}

void OpenAi2::_onInternalCallSuccess()
{
    m_consecutiveHardFailures = 0;
}

void OpenAi2::_onInternalCallError(const QString &err)
{
    // Hard failure rule: fatal, 401, 403.
    // Assuming error string starts with these or contains them.
    // Simple heuristic for now based on what Real transport might return.
    bool isHard = err.startsWith("fatal:") || err.contains("401") || err.contains("403");
    
    if (isHard)
    {
        m_consecutiveHardFailures++;
        if (m_consecutiveHardFailures >= 5) // circuit breaker threshold
        {
             // m_blockedUntilMs = QDateTime::currentMSecsSinceEpoch() + 60000;
             // Logic kept simple for now as requested.
        }
    }
}

void OpenAi2::_callResponses_Real(const QString &model,
                             const QString &prompt,
                             const QList<QString> &imagePaths,
                             std::function<void(QString raw)> onOk,
                             std::function<void(QString err)> onErr)
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

    // Optional circuit-breaker (requires members):
    // qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    // if (m_blockedUntilMs > nowMs)
    // {
    //     if (onErr)
    //     {
    //         onErr("circuit_breaker_blocked");
    //     }
    //     return;
    // }

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
                onErr(QString("cannot_open_image:%1").arg(path));
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
    req.setRawHeader("Authorization", auth.toUtf8());
    req.setRawHeader("Accept", "application/json");
    req.setRawHeader("User-Agent", "OpenAi2-QtTest/1.0");

    // IMPORTANT: throttle/schedule through _schedule
    bool isImage = false;
    isImage = !imagePaths.isEmpty();

    std::function<void(std::function<void()>)> startRequest;
    startRequest = [this, req, payload, onOk, onErr](std::function<void()> done)
    {
        QNetworkReply *reply = nullptr;
        reply = m_networkAccessManager.post(req, payload);

        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, onOk, onErr, done]()
        {
            QByteArray body;
            body = reply->readAll();

            int httpStatus = 0;
            httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            QNetworkReply::NetworkError netErr = reply->error();
            const QVariant wwwAuth = reply->rawHeader("www-authenticate");
            qWarning() << "HTTP status:" << httpStatus
                       << "netErr:" << netErr
                       << "url:" << reply->url()
                       << "www-authenticate:" << wwwAuth.toString();
            qWarning() << "Body (first 1024):" << QString::fromUtf8(body.left(1024));

            reply->deleteLater();

            if (netErr != QNetworkReply::NoError)
            {
                QString err;
                err = QString("network_error:%1:http:%2:%3")
                          .arg(static_cast<int>(netErr))
                          .arg(httpStatus)
                          .arg(QString::fromUtf8(body.left(512)));

                // Network error is a "hard" failure (can be retryable usually, but if consecutive fails stack up...)
                // For now, logic: keep counting.
                m_consecutiveHardFailures++;

                if (onErr)
                {
                    onErr(err);
                }
                return;
            }

            if (httpStatus >= 400)
            {
                QString err;
                // Client errors (400, 401, 403) are typically fatal (logic error, auth error)
                // 429 (Too Many Requests) is retryable (throttling)
                // 5xx (Server Error) is retryable
                
                bool isFatal = (httpStatus == 400 || httpStatus == 401 || httpStatus == 403);
                
                if (isFatal) {
                     err = QString("fatal:http_error:%1:%2")
                          .arg(httpStatus)
                          .arg(QString::fromUtf8(body.left(1024)));
                } else {
                     err = QString("http_error:%1:%2")
                          .arg(httpStatus)
                          .arg(QString::fromUtf8(body.left(1024)));
                }

                if (httpStatus == 429)
                {
                     m_blockedUntilMs = QDateTime::currentMSecsSinceEpoch() + 2 * 60 * 1000;
                }

                // If fatal, we might consider it a harder fail?
                m_consecutiveHardFailures++;

                if (onErr)
                {
                    onErr(err);
                }
                return;
            }

            // Success
            m_consecutiveHardFailures = 0;

            // Return RAW JSON string (caller validate/apply decides what to do)
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
                if (onErr) onErr("parse_error:no_output_text");
                if (done) done();
                return;
            }

            if (onOk)
            {
                onOk(text);
            }
            if (done) done();


        });
    };

    // Route to scheduler
    this->_schedule(startRequest);

    Q_UNUSED(isImage);
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

void OpenAi2::_schedule(std::function<void(std::function<void()>)> startRequest)
{
    // REQUIREMENT: no static variables. This scheduler assumes MEMBER state exists.
    // Add in header:
    //   QQueue<std::function<void()>> m_pendingText;
    //   int m_inFlightText;
    //   bool m_pumping;
    // Also add image queue/limits if you want separate throttling by type.

    if (!startRequest)
    {
        return;
    }

    m_pendingText.enqueue(startRequest);

    if (m_pumping)
    {
        return;
    }

    m_pumping = true;

    QSharedPointer<std::function<void()>> pump = QSharedPointer<std::function<void()>>::create();
    (*pump) = [this, pump]()
    {
        bool progressed = false;
        progressed = false;

        while (!m_pendingText.isEmpty())
        {
            if (m_inFlightText >= m_maxQueriesSameTime)
            {
                break;
            }

            std::function<void(std::function<void()>)> job;
            job = m_pendingText.dequeue();

            m_inFlightText = m_inFlightText + 1;
            progressed = true;

            QTimer::singleShot(0, this, [this, job, pump]()
            {
                try
                {
                    // Pass a "done" callback that decrements inFlight and re-pumps
                    job([this, pump](){
                        m_inFlightText = m_inFlightText - 1;
                        if (m_inFlightText < 0) m_inFlightText = 0;
                        
                        // Trigger pump again to consume next in queue if any
                         QTimer::singleShot(0, this, [pump](){ (*pump)(); });
                    });
                }
                catch (...)
                {
                    m_inFlightText = m_inFlightText - 1; 
                }
            });

            // NO immediate decrement here. It happens in the done() callback.

            // Removed the immediate decrement logic block
        }

        m_pumping = false;

        if (!progressed)
        {
            int delay = 0;
            delay = 50;
            QTimer::singleShot(delay, this, [this, pump]()
            {
                (*pump)();
            });
        }
        else
        {
            if (!m_pendingText.isEmpty())
            {
                QTimer::singleShot(0, this, [this, pump]()
                {
                    (*pump)();
                });
            }
        }
    };

    (*pump)();
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
        this->_runStepWithRetries(
            step,
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
            [&err, &ok, &loop](QString e)
            {
                err = e;
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
