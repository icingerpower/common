#include <QDateTime>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QByteArray>
#include <QUrl>
#include <QUuid>

#include "ExceptionOpenAiNotInitialized.h"

#include "OpenAi.h"


#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QBuffer>
#include <QByteArray>
#include <QUrl>
#include <QThread>
#include <QUuid>

#include "OpenAi.h"
#include "ExceptionOpenAiNotInitialized.h"

const QString OpenAi::KEY_OPEN_AI_API_KEY = "open-ai-api-key";
const QUrl OpenAi::RESPONSES_URL("https://api.openai.com/v1/responses");
//const QUrl OpenAi::RESPONSES_URL("https://api.openai.com/v1/chat/completions");

// -------- ctor/dtor/singleton ----------
OpenAi::OpenAi(QObject *parent) : QObject(parent)
{
}

OpenAi::~OpenAi() = default;

OpenAi* OpenAi::instance() {
    static OpenAi inst;
    return &inst;
}

bool OpenAi::isInitiatized() const {
    return m_initialized;
}

void OpenAi::init(const QString &openAiKey) {
    m_openAiKey = openAiKey;
    m_initialized = true;
}

void OpenAi::_raiseExceptionIfNotInitialized() {
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized ex; ex.raise();
    }
}

// -------- throughput controls ----------
void OpenAi::setMaxInFlight(int n)
{
    m_maxInFlight = qMax(1, n);
    _scheduleDispatch(0);
}

void OpenAi::setMinSpacingMs(int ms)
{
    m_minSpacingMs = qMax(0, ms);
}

int  OpenAi::maxInFlight() const
{
    return m_maxInFlight;
}

int  OpenAi::minSpacingMs() const
{
    return m_minSpacingMs;
}

// -------- timeouts/retries -------------
int  OpenAi::maxRetries() const
{
    return m_maxRetries;
}

void OpenAi::setMaxRetries(int newMaxRetries)
{
    m_maxRetries = qMax(0, newMaxRetries);
}

int  OpenAi::timeoutMs() const
{
    return m_timeoutMs;
}

void OpenAi::setTimeoutMs(int newTimeoutMs)
{
    m_timeoutMs = qMax(1000, newTimeoutMs);
}

// -------- prompt caching ---------------
void OpenAi::setCachedPrefix(const QString& cachingKey, const QString& prefixText)
{
    if (!cachingKey.isEmpty()) m_cachedPrefixByKey.insert(cachingKey, prefixText);
}

void OpenAi::clearCachedPrefix(const QString& cachingKey) {
    m_cachedPrefixByKey.remove(cachingKey);
}

bool OpenAi::hasCachedPrefix(const QString& cachingKey) const
{
    return m_cachedPrefixByKey.contains(cachingKey);
}

QString OpenAi::cachedPrefix(const QString& cachingKey) const
{
    return m_cachedPrefixByKey.value(cachingKey);
}

QNetworkAccessManager *OpenAi::_netForAttempt(int attempt)
{
    auto it = m_attempt_networkAccessManager.find(attempt);
    if (it != m_attempt_networkAccessManager.end())
    {
        return it.value();
    }
    auto *net = new QNetworkAccessManager(this);
    m_attempt_networkAccessManager.insert(attempt, net);
    return net;
}

// -------- public askQuestion (text) ----
void OpenAi::askQuestion(const QString& question,
    const QString& cachingKey,
    int nMaxRetryOnReplyFailed,
    const QString& model,
    std::function<bool(const QString& json)> callbackTryProcessReply,
    std::function<void(const QString& json)> callbackReplySuccess,
    std::function<void(const QString& json)> callbackReplyFailure)
{
    _raiseExceptionIfNotInitialized();
    Pending p;
    p.question = question;
    p.cachingKey = cachingKey;
    p.model = model;
    p.image = QImage();
    p.callbackTry = std::move(callbackTryProcessReply);
    p.callbackSuccess = std::move(callbackReplySuccess);
    p.callbackFailure = std::move(callbackReplyFailure);
    p.semanticRetries = qMax(0, nMaxRetryOnReplyFailed);
    _enqueue(p);
}

// -------- public askQuestion (image) ---
void OpenAi::askQuestion(const QString& question,
    const QImage& image,
    const QString& cachingKey,
    int nMaxRetryOnReplyFailed,
    const QString& model,
    std::function<bool(const QString& json)> callbackTryProcessReply,
    std::function<void(const QString& json)> callbackReplySuccess,
    std::function<void(const QString& json)> callbackReplyFailure)
{
    _raiseExceptionIfNotInitialized();
    Pending p;
    p.question = question;
    p.cachingKey = cachingKey;
    p.model = model;
    p.image = image;
    p.callbackTry = std::move(callbackTryProcessReply);
    p.callbackSuccess = std::move(callbackReplySuccess);
    p.callbackFailure = std::move(callbackReplyFailure);
    p.semanticRetries = qMax(0, nMaxRetryOnReplyFailed);
    _enqueue(p);
}

// -------- queueing ---------------------
void OpenAi::_enqueue(const Pending& p)
{
    m_queue.enqueue(p);
    _scheduleDispatch(0);
}

void OpenAi::_scheduleDispatch(int delayMs)
{
    if (m_dispatchScheduled)
    {
        return;
    }
    m_dispatchScheduled = true;
    QTimer::singleShot(delayMs, this, [this]{
        m_dispatchScheduled = false;
        _dispatchOne();
    });
}

void OpenAi::_dispatchOne() {
    while (m_inFlight < m_maxInFlight && !m_queue.isEmpty())
    {
        const Pending p = m_queue.dequeue();

        auto* ctx = new InFlight;
        ctx->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        ctx->model = p.model;
        ctx->cachingKey = p.cachingKey;
        ctx->question = p.question;
        ctx->callbackTry = p.callbackTry;
        ctx->callbackSuccess = p.callbackSuccess;
        ctx->callbackFailure = p.callbackFailure;
        ctx->semanticRetriesLeft = p.semanticRetries;
        ctx->callerThread = QThread::currentThread();
        ctx->proxy = new QObject();
        ctx->proxy->moveToThread(ctx->callerThread);
        if (!p.image.isNull())
        {
            ctx->imagePngBase64 = _toPngBase64(p.image);
        }

        ++m_inFlight;
        //_send(ctx);
        _sendWithPacing(ctx, 0);

        if (!m_queue.isEmpty())
        {
            _scheduleDispatch(m_minSpacingMs); return;
        }
    }
}
void OpenAi::_sendWithPacing(InFlight* ctx, int minDelayMs)
{
    int wait = qMax(0, minDelayMs);
    if (!m_lastSendInit) {
        m_lastSend.start();
        m_lastSendInit = true;
    } else {
        const int sinceLast = (int)m_lastSend.elapsed();
        // Enforce global spacing
        wait = qMax(wait, m_minSpacingMs - sinceLast);
    }

    if (wait <= 0) {
        m_lastSend.restart();
        _send(ctx);
    } else {
        QTimer::singleShot(wait, this, [this, ctx]{
            m_lastSend.restart();
            _send(ctx);
        });
    }
}
// -------- network ----------------------
void OpenAi::_send(InFlight* ctx)
{
    QNetworkRequest req(RESPONSES_URL);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ").append(m_openAiKey.toUtf8()));
    req.setRawHeader("Accept", "application/json");

    const QByteArray body = _buildBody(ctx);
    qDebug() << "OpenAi posting..." << QDateTime::currentDateTime().toString("hh:MM:ss zzz");
    static QDateTime lastDateTime = QDateTime::currentDateTime().addMSecs(-m_minSpacingMs-10000);
    QDateTime currentDateTime = QDateTime::currentDateTime();
    auto ellapsed = lastDateTime.msecsTo(currentDateTime);
    Q_ASSERT(m_maxInFlight > 1 ||ellapsed > m_minSpacingMs * 0.9);
    lastDateTime = currentDateTime;

    ctx->reply = _netForAttempt(ctx->attempt)->post(req, body);

    ctx->timer = new QTimer(this);
    ctx->timer->setSingleShot(true);
    connect(ctx->timer, &QTimer::timeout, this, [this, ctx]{
        if (ctx->reply && ctx->reply->isRunning())
        {
            ctx->reply->abort();
        }
    });
    ctx->timer->start(m_timeoutMs);

    connect(ctx->reply, &QNetworkReply::finished, this, [this, ctx]{
        ctx->timer->stop();

        const int http = ctx->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QString &retryAfter = QString::fromLatin1(ctx->reply->rawHeader("Retry-After"));
        const QString &errStr = ctx->reply->errorString();
        ctx->lastRaw = ctx->reply->readAll();

        ///auto parseOpenAiError = [&]() -> QString {
        auto parseOpenAiError = [&ctx, errStr]() -> QString {
            QJsonParseError perr{}; QJsonDocument d = QJsonDocument::fromJson(ctx->lastRaw, &perr);
            if (perr.error == QJsonParseError::NoError && d.isObject())
            {
                const QJsonObject o = d.object();
                if (o.contains("error") && o["error"].isObject())
                {
                    const QJsonObject e = o["error"].toObject();
                    const QString em = e.value("message").toString();
                    const QString et = e.value("type").toString();
                    if (!em.isEmpty())
                    {
                        return QString("[%1] %2").arg(et, em);
                    }
                }
            }
            return errStr;
        };

        if (ctx->reply->error() != QNetworkReply::NoError)
        {
            const bool retry = _shouldRetry(ctx->reply) && ctx->attempt < m_maxRetries;
            ctx->reply->deleteLater();
            ctx->reply = nullptr;
            if (retry)
            {
                _retry(ctx, retryAfter); return;
            }

            QString msg = parseOpenAiError();
            if (http)
            {
                msg = QString("HTTP %1 — %2").arg(http).arg(msg);
            }
            if (!retryAfter.isEmpty())
            {
                msg += QString(" (Retry-After: %1s)").arg(retryAfter);
            }
            //qDebug() << "Open AI failure: " << msg;
            _postFailure(ctx, msg);
            _finalize(ctx, false, msg);
            return;
        }


        ctx->reply->deleteLater();
        ctx->reply = nullptr;

        // Parse success (Responses API)
        QJsonParseError perr{};
        QJsonDocument doc = QJsonDocument::fromJson(ctx->lastRaw, &perr);
        if (perr.error != QJsonParseError::NoError || !doc.isObject())
        {
            QString m = QStringLiteral("Invalid JSON response from OpenAI.");
            _postFailure(ctx, m);
            _finalize(ctx, false, m);
            return;
        }
        const QJsonObject root = doc.object();

        QString outText;
        if (root.contains("output") && root.value("output").isArray())
        {
            const auto outArr = root.value("output").toArray();
            if (!outArr.isEmpty() && outArr.at(0).isObject())
            {
                const auto o0 = outArr.at(0).toObject();
                if (o0.contains("content") && o0.value("content").isArray())
                {
                    const auto cArr = o0.value("content").toArray();
                    for (const auto& c : cArr)
                    {
                        const auto o = c.toObject();
                        if (o.value("type").toString() == "output_text")
                        {
                            outText += o.value("text").toString();
                        }
                    }
                }
            }
        }
        if (outText.isEmpty() && root.contains("output_text"))
        {
            outText = root.value("output_text").toString();
        }

        /*
        if (outText.isEmpty())
        {
            QString m = QStringLiteral("Empty response from OpenAI.");
            _postFailure(ctx, m);
            _finalize(ctx, false, m);
            return;
        }
        //*/

        // Semantic validation/retry
        if (ctx->callbackTry)
        {
            QString candidate = outText; // pass by value, allow mutation in callbackTry
            bool ok = false;
            try { ok = ctx->callbackTry(candidate); } catch (...) { ok = false; }

            if (!ok)
            {
                if (ctx->semanticRetriesLeft > 0)
                {
                    ctx->semanticRetriesLeft--;
                    //const int delay = qMax(200, m_minSpacingMs);
                    // small delay to decorrelate completions
                    //QTimer::singleShot(delay, this, [this, ctx]{ _send(ctx); });
                    _sendWithPacing(ctx, 200);
                    return; // keep slot
                }
                else
                {
                    _postFailure(ctx, candidate); // provide last bad reply
                    _finalize(ctx, false, QStringLiteral("Validation failed and retries exhausted."));
                    return;
                }
            }

            // accepted
            _postSuccess(ctx, candidate);
            _finalize(ctx, true, QString());
            return;
        }

        // No validator, deliver as-is
        QString deliver = outText;
        _postSuccess(ctx, deliver);
        _finalize(ctx, true, QString());
    });
}

QByteArray OpenAi::_buildBody(const InFlight* ctx) const
{
    QJsonArray input;

    // SYSTEM (stable; use input_text)
    input.append(QJsonObject{
        {"role","system"},
        {"content", QJsonArray{
                        QJsonObject{{"type","input_text"},{"text","You are a helpful assistant. Reply succinctly."}}
                    }}
    });

    // Cached prefix (if any) as input_text
    const bool havePrefix = !ctx->cachingKey.isEmpty() && m_cachedPrefixByKey.contains(ctx->cachingKey);
    if (havePrefix)
    {
        input.append(QJsonObject{
            {"role","user"},
            {"content", QJsonArray{
                            QJsonObject{{"type","input_text"},{"text", m_cachedPrefixByKey.value(ctx->cachingKey)}}
                        }}
        });
    }

    // Current question (+ optional image via data URL)
    QJsonArray qContent{
        QJsonObject{{"type","input_text"},{"text", ctx->question}}
    };
    if (!ctx->imagePngBase64.isEmpty())
    {
        const QString dataUrl = _imageDataUrlFromBase64Png(ctx->imagePngBase64);
        qContent.append(QJsonObject{
            {"type","input_image"},
            {"image_url", dataUrl}
        });
    }
    input.append(QJsonObject{{"role","user"},{"content", qContent}});

    QJsonObject body{{"model", ctx->model}, {"input", input}};
    if (!ctx->cachingKey.isEmpty())
    {
        body["prompt_cache_key"] = ctx->cachingKey;
    }
    // If you need strict JSON outputs, add response_format: { type: "json_schema", ... } here.
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

// -------- helpers ----------------------
void OpenAi::_retry(InFlight* ctx, const QString& retryAfterHeader)
{
    _netForAttempt(ctx->attempt)->clearConnectionCache();
    ctx->attempt++;
    qDebug() << "Open Ai clearing cache after error";
    int backoffMs = m_minSpacingMs * (1 << (ctx->attempt - 1)); // 500, 1000, 2000...
    bool ok=false; int ra = retryAfterHeader.toInt(&ok);
    if (ok)
    {
        backoffMs = qMax(backoffMs, ra * 1000);
    }
    _sendWithPacing(ctx, backoffMs);
    //QTimer::singleShot(backoffMs, this, [this, ctx]{
        //_send(ctx);
    //});
}

bool OpenAi::_shouldRetry(QNetworkReply* r) const
{
    const int code = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return (code == 429 || (code >= 500 && code < 600));
}

void OpenAi::_finalize(InFlight* ctx, bool ok, const QString& err)
{
    if (ctx->timer)
    {
        ctx->timer->deleteLater(); ctx->timer = nullptr;
    }

    m_inFlight = qMax(0, m_inFlight - 1);
    _scheduleDispatch(m_minSpacingMs);

    // Delete proxy on its own thread
    if (ctx->proxy)
    {
        QObject* proxy = ctx->proxy;
        ctx->proxy = nullptr;
        QMetaObject::invokeMethod(proxy, [proxy]{ proxy->deleteLater(); }, Qt::QueuedConnection);
    }

    if (!ok)
    {
        ctx->callbackFailure(err);
        //ExceptionOpenAiError ex; ex.setError(err); ex.raise();
    }
    delete ctx;
}

void OpenAi::_postSuccess(InFlight* ctx, const QString& text)
{
    if (!ctx->callbackSuccess)
    {
        return;
    }
    auto callback = ctx->callbackSuccess;
    QString s = text;
    if (QThread::currentThread() == ctx->callerThread)
    {
        callback(s);
    }
    else
    {
        QMetaObject::invokeMethod(ctx->proxy, [callback, s]() mutable {
            QString x = s;
            callback(x);
        }, Qt::QueuedConnection);
    }
}

void OpenAi::_postFailure(InFlight* ctx, const QString& jsonOrErr)
{
    if (!ctx->callbackFailure) return;
    auto callback = ctx->callbackFailure;
    QString s = jsonOrErr;
    if (QThread::currentThread() == ctx->callerThread)
    {
        callback(s);
    }
    else
    {
        QMetaObject::invokeMethod(ctx->proxy, [callback, s]() mutable {
            QString x = s;
            callback(x);
        }, Qt::QueuedConnection);
    }
}

QByteArray OpenAi::_toPngBase64(const QImage& img)
{
    QByteArray raw;
    QBuffer buf(&raw);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    buf.close();
    return raw.toBase64();
}

QString OpenAi::_imageDataUrlFromBase64Png(const QByteArray& b64png)
{
    return "data:image/png;base64," + QString::fromLatin1(b64png);
}

