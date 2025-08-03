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
#include "ExceptionOpenAiError.h"

#include "OpenAi.h"


const QString OpenAi::KEY_OPEN_AI_API_KEY = "open-ai-api-key";
const QUrl OpenAi::RESPONSES_URL("https://api.openai.com/v1/responses");

OpenAi::OpenAi(QObject *parent) : QObject(parent)
{
    m_net = new QNetworkAccessManager(this);
}

OpenAi::~OpenAi() = default;

OpenAi *OpenAi::instance()
{
    static OpenAi instance;
    return &instance;
}

bool OpenAi::isInitiatized() const { return m_initialized; }

void OpenAi::init(const QString &openAiKey)
{
    m_openAiKey = openAiKey;
    m_initialized = true;
}

// --------------------- Public API ---------------------

void OpenAi::askQuestion(const QString &question,
                         const QString &cachingKey,
                         std::function<void (QString &)> callbackReply,
                         const QString &model)
{
    _raiseExceptionIfNotInitialized();
    Pending p{question, cachingKey, QImage(), model, std::move(callbackReply)};
    _enqueue(p);
}

void OpenAi::askQuestion(const QString &question,
                         const QImage &image,
                         const QString &cachingKey,
                         std::function<void (QString &)> callbackReply,
                         const QString &model)
{
    _raiseExceptionIfNotInitialized();
    Pending p{question, cachingKey, image, model, std::move(callbackReply)};
    _enqueue(p);
}

// Throughput
void OpenAi::setMaxInFlight(int n) { m_maxInFlight = qMax(1, n); _scheduleDispatch(0); }
void OpenAi::setMinSpacingMs(int ms) { m_minSpacingMs = qMax(0, ms); }
int  OpenAi::maxInFlight() const { return m_maxInFlight; }
int  OpenAi::minSpacingMs() const { return m_minSpacingMs; }

// Timeouts / retries
int  OpenAi::maxRetries() const { return m_maxRetries; }
void OpenAi::setMaxRetries(int newMaxRetries) { m_maxRetries = qMax(0, newMaxRetries); }
int  OpenAi::timeoutMs() const { return m_timeoutMs; }
void OpenAi::setTimeoutMs(int newTimeoutMs) { m_timeoutMs = qMax(1000, newTimeoutMs); }

// Prompt-caching control
void OpenAi::setCachedPrefix(const QString& cachingKey, const QString& prefixText)
{
    if (cachingKey.isEmpty()) return;
    m_cachedPrefixByKey.insert(cachingKey, prefixText);
}
void OpenAi::clearCachedPrefix(const QString& cachingKey) { m_cachedPrefixByKey.remove(cachingKey); }
bool OpenAi::hasCachedPrefix(const QString& cachingKey) const { return m_cachedPrefixByKey.contains(cachingKey); }
QString OpenAi::cachedPrefix(const QString& cachingKey) const { return m_cachedPrefixByKey.value(cachingKey); }

// --------------------- Queueing ---------------------

void OpenAi::_enqueue(const Pending& p)
{
    m_queue.enqueue(p);
    _scheduleDispatch(0);
}

void OpenAi::_scheduleDispatch(int delayMs)
{
    if (m_dispatchScheduled) return;
    m_dispatchScheduled = true;
    QTimer::singleShot(delayMs, this, [this]{
        m_dispatchScheduled = false;
        _dispatchOne();
    });
}

void OpenAi::_dispatchOne()
{
    while (m_inFlight < m_maxInFlight && !m_queue.isEmpty())
    {
        const Pending p = m_queue.dequeue();

        auto* ctx = new InFlight;
        ctx->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        ctx->model = p.model;
        ctx->cachingKey = p.cachingKey;
        ctx->question = p.question;
        ctx->callback = p.cb;
        ctx->attempt = 0;
        if (!p.image.isNull()) ctx->imagePngBase64 = _toPngBase64(p.image);

        ++m_inFlight;
        _send(ctx);

        // Smooth the burst; schedule next tick
        if (!m_queue.isEmpty()) { _scheduleDispatch(m_minSpacingMs); return; }
    }
}

// --------------------- Network ---------------------

void OpenAi::_send(InFlight* ctx)
{
    QNetworkRequest req(RESPONSES_URL);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ").append(m_openAiKey.toUtf8()));
    req.setRawHeader("Accept", "application/json");

    const QByteArray body = _buildBody(ctx);
    ctx->reply = m_net->post(req, body);

    ctx->timer = new QTimer(this);
    ctx->timer->setSingleShot(true);
    connect(ctx->timer, &QTimer::timeout, this, [this, ctx]{
        if (ctx->reply && ctx->reply->isRunning()) ctx->reply->abort();
    });
    ctx->timer->start(m_timeoutMs);

    connect(ctx->reply, &QNetworkReply::finished, this, [this, ctx]{
        ctx->timer->stop();

        const int http = ctx->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QString retryAfter = QString::fromLatin1(ctx->reply->rawHeader("Retry-After"));
        const QByteArray raw = ctx->reply->readAll();

        auto parseOpenAiError = [raw, ctx]() -> QString {
            QJsonParseError perr{};
            QJsonDocument d = QJsonDocument::fromJson(raw, &perr);
            if (perr.error == QJsonParseError::NoError && d.isObject()) {
                const QJsonObject o = d.object();
                if (o.contains("error") && o["error"].isObject()) {
                    const QJsonObject e = o["error"].toObject();
                    const QString em = e.value("message").toString();
                    const QString et = e.value("type").toString();
                    if (!em.isEmpty()) return QString("[%1] %2").arg(et, em);
                }
            }
            return ctx->reply->errorString();
        };

        if (ctx->reply->error() != QNetworkReply::NoError) {
            const bool retry = _shouldRetry(ctx->reply) && ctx->attempt < m_maxRetries;
            ctx->reply->deleteLater(); ctx->reply = nullptr;
            if (retry) { _retry(ctx, retryAfter); return; }
            QString msg = parseOpenAiError();
            if (http) msg = QString("HTTP %1 — %2").arg(http).arg(msg);
            if (!retryAfter.isEmpty()) msg += QString(" (Retry-After: %1s)").arg(retryAfter);
            _finalize(ctx, false, msg);
            return;
        }

        ctx->reply->deleteLater(); ctx->reply = nullptr;

        // Parse success
        QJsonParseError perr{};
        QJsonDocument doc = QJsonDocument::fromJson(raw, &perr);
        if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
            _finalize(ctx, false, "Invalid JSON response from OpenAI.");
            return;
        }
        const QJsonObject root = doc.object();

        // Extract assistant text (Responses API)
        QString outText;
        if (root.contains("output") && root.value("output").isArray()) {
            const auto outArr = root.value("output").toArray();
            if (!outArr.isEmpty() && outArr.at(0).isObject()) {
                const auto o0 = outArr.at(0).toObject();
                if (o0.contains("content") && o0.value("content").isArray()) {
                    const auto cArr = o0.value("content").toArray();
                    for (const auto& c : cArr) {
                        const auto o = c.toObject();
                        if (o.value("type").toString() == "output_text")
                            outText += o.value("text").toString();
                    }
                }
            }
        }
        if (outText.isEmpty() && root.contains("output_text"))
            outText = root.value("output_text").toString();

        if (outText.isEmpty()) {
            _finalize(ctx, false, "Empty response from OpenAI.");
            return;
        }

        _finalize(ctx, true, QString(), outText);
    });
}

QByteArray OpenAi::_buildBody(const InFlight* ctx) const
{
    QJsonArray input;

    // Keep system stable if you want it in the cached prefix
    input.append(QJsonObject{
        {"role","system"},
        {"content","You are a helpful assistant. Reply succinctly."}
    });

    // Shared cached prefix (if present)
    const bool havePrefix = !ctx->cachingKey.isEmpty() && m_cachedPrefixByKey.contains(ctx->cachingKey);
    if (havePrefix) {
        input.append(QJsonObject{
            {"role","user"},
            {"content", QJsonArray{
                QJsonObject{{"type","input_text"},{"text", m_cachedPrefixByKey.value(ctx->cachingKey)}}
            }}
        });
    }

    // Current question (and optional image)
    QJsonArray qContent{
        QJsonObject{{"type","input_text"},{"text", ctx->question}}
    };
    if (!ctx->imagePngBase64.isEmpty()) {
        qContent.append(QJsonObject{
            {"type","input_image"},
            {"image_data", QString::fromLatin1(ctx->imagePngBase64)},
            {"mime_type", _mimePNG()}
        });
    }
    input.append(QJsonObject{{"role","user"},{"content", qContent}});

    QJsonObject body{{"model", ctx->model}, {"input", input}};
    // If you need strict JSON outputs, put response_format: json_schema here.
    return QJsonDocument(body).toJson(QJsonDocument::Compact);
}

// --------------------- Helpers ---------------------

void OpenAi::_retry(InFlight* ctx, const QString& retryAfterHeader)
{
    ctx->attempt++;
    int backoffMs = 500 * (1 << (ctx->attempt - 1)); // 500, 1000, 2000...
    bool ok=false; int ra = retryAfterHeader.toInt(&ok);
    if (ok) backoffMs = qMax(backoffMs, ra * 1000);
    QTimer::singleShot(backoffMs, this, [this, ctx]{ _send(ctx); });
}

bool OpenAi::_shouldRetry(QNetworkReply* r) const
{
    const int code = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return (code == 429 || (code >= 500 && code < 600));
}

void OpenAi::_finalize(InFlight* ctx, bool ok, const QString& err, const QString& text)
{
    if (ctx->timer) { ctx->timer->deleteLater(); ctx->timer = nullptr; }

    // Decrement in-flight and schedule next
    m_inFlight = qMax(0, m_inFlight - 1);
    _scheduleDispatch(m_minSpacingMs);

    if (!ok) {
        ExceptionOpenAiError ex;
        ex.setError(err);
        ex.raise(); // matches your exception pattern
        delete ctx;
        return;
    }

    if (ctx->callback) {
        QString out = text;
        ctx->callback(out);
    }
    delete ctx;
}

QString OpenAi::_mimePNG() { return "image/png"; }

QByteArray OpenAi::_toPngBase64(const QImage& img)
{
    QByteArray raw;
    QBuffer buf(&raw);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    buf.close();
    return raw.toBase64();
}

void OpenAi::_raiseExceptionIfNotInitialized()
{
    if (!m_initialized) {
        ExceptionOpenAiNotInitialized ex;
        ex.raise();
    }
}



/*
const QString OpenAi::KEY_OPEN_AI_API_KEY = "open-ai-api-key";
const QUrl OpenAi::RESPONSES_URL("https://api.openai.com/v1/responses");

OpenAi::OpenAi(QObject *parent) : QObject(parent)
{
    m_initialized = false;
    m_maxRetries = 3;
    m_timeoutMs = 30000;
    m_net = new QNetworkAccessManager();
}

void OpenAi::_raiseExceptionIfNotInitialized()
{
    if (!m_initialized)
    {
        ExceptionOpenAiNotInitialized exception;
        exception.raise();
    }
}

OpenAi::~OpenAi()
{
    delete m_net;
}

OpenAi *OpenAi::instance()
{
    static OpenAi instance;
    return &instance;
}

bool OpenAi::isInitiatized() const
{
    return m_initialized;
}

void OpenAi::init(const QString &openAiKey)
{
    m_openAiKey = openAiKey;
    m_initialized = true;
}

void OpenAi::askQuestion(const QString &question,
                         const QString &cachingKey,
                         std::function<void (QString &)> callbackReply,
                         const QString &model)
{
    _raiseExceptionIfNotInitialized();
    auto* ctx = new InFlight;
    ctx->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ctx->model = model;
    ctx->cachingKey = cachingKey;
    ctx->question = question;
    ctx->callback = std::move(callbackReply);
    ctx->attempt = 0;

    _send(ctx);
}

void OpenAi::askQuestion(const QString &question,
                         const QImage &image,
                         const QString &cachingKey,
                         std::function<void (QString &)> callbackReply,
                         const QString &model)
{
    _raiseExceptionIfNotInitialized();
    auto* ctx = new InFlight;
    ctx->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ctx->model = model;
    ctx->cachingKey = cachingKey;
    ctx->question = question;
    ctx->callback = std::move(callbackReply);
    ctx->attempt = 0;

    if (!image.isNull())
    {
        ctx->imagePngBase64 = _toPngBase64(image);
    }

    _send(ctx);
}

int OpenAi::timeoutMs() const
{
    return m_timeoutMs;
}

void OpenAi::setTimeoutMs(int newTimeoutMs)
{
    m_timeoutMs = newTimeoutMs;
}

int OpenAi::maxRetries() const
{
    return m_maxRetries;
}

void OpenAi::setMaxRetries(int newMaxRetries)
{
    m_maxRetries = newMaxRetries;
}

void OpenAi::_send(InFlight* ctx)
{
    // Build HTTP request
    QNetworkRequest req(RESPONSES_URL);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ").append(m_openAiKey.toUtf8()));

    // Build body (Responses API, non-streaming)
    QByteArray body = _buildBody(ctx);

    // Dispatch
    ctx->reply = m_net->post(req, body);

    // Timeout
    ctx->timer = new QTimer(this);
    ctx->timer->setSingleShot(true);
    connect(ctx->timer, &QTimer::timeout, this, [this, ctx]{
        if (ctx->reply && ctx->reply->isRunning())
            ctx->reply->abort();
    });
    ctx->timer->start(m_timeoutMs);

    // Finish handler
    connect(ctx->reply, &QNetworkReply::finished, this, [this, ctx]{
        ctx->timer->stop();

        if (ctx->reply->error() != QNetworkReply::NoError)
        {
            const bool retry = _shouldRetry(ctx->reply) && ctx->attempt < m_maxRetries;
            const QString err = ctx->reply->errorString();
            ctx->reply->deleteLater();
            ctx->reply = nullptr;

            if (retry) { _retry(ctx); return; }

            _finalize(ctx, false, err);
            return;
        }

        const QByteArray resp = ctx->reply->readAll();
        ctx->reply->deleteLater();
        ctx->reply = nullptr;

        // Parse Responses API JSON
        QJsonParseError perr{};
        QJsonDocument doc = QJsonDocument::fromJson(resp, &perr);
        if (perr.error != QJsonParseError::NoError || !doc.isObject())
        {
            _finalize(ctx, false, "Invalid JSON response from OpenAI.");
            return;
        }

        const QJsonObject root = doc.object();

        // Extract assistant text:
        // Try: output[0].content[*].text (Responses API)
        QString outText;
        if (root.contains("output") && root.value("output").isArray())
        {
            const QJsonArray outArr = root.value("output").toArray();
            if (!outArr.isEmpty() && outArr.at(0).isObject())
            {
                const QJsonObject o0 = outArr.at(0).toObject();
                if (o0.contains("content") && o0.value("content").isArray())
                {
                    const QJsonArray cArr = o0.value("content").toArray();
                    for (const auto& c : cArr)
                    {
                        if (!c.isObject()) continue;
                        const QJsonObject cObj = c.toObject();
                        if (cObj.value("type").toString() == "output_text")
                        {
                            outText += cObj.value("text").toString();
                        }
                    }
                }
            }
        }

        if (outText.isEmpty())
        {
            // Fallbacks (defensive)
            if (root.contains("output_text"))
                outText = root.value("output_text").toString();
        }

        if (outText.isEmpty())
        {
            _finalize(ctx, false, "Empty response from OpenAI.");
            return;
        }

        // On first successful call for a cachingKey, store the prefix (the first question)
        // This makes subsequent calls reuse the exact same prefix bytes.
        if (!ctx->cachingKey.isEmpty() && !m_cachedPrefixByKey.contains(ctx->cachingKey))
        {
            m_cachedPrefixByKey.insert(ctx->cachingKey, ctx->question);
        }

        _finalize(ctx, true, QString(), outText);
    });
}

QByteArray OpenAi::_buildBody(const InFlight* ctx) const
{
    // Build "input" array for the Responses API.
    // If we already have a cached prefix for this key, we send two user messages:
    //   1) cached prefix text (identical bytes across calls)
    //   2) current question (the small changing suffix)
    // If no cached prefix yet, we send only the current question (and will cache it on success).

    QJsonArray input;

    // (Optional) You can add a short system message to enforce JSON-only, etc.
    // Keep it stable if you want it to be part of the cached prefix.
    // Here we keep it minimal and constant.
    {
        QJsonObject sys;
        sys["role"] = "system";
        sys["content"] = "You are a helpful assistant. Reply succinctly.";
        input.append(sys);
    }

    const bool havePrefix = !ctx->cachingKey.isEmpty() && m_cachedPrefixByKey.contains(ctx->cachingKey);

    if (havePrefix)
    {
        QJsonObject u1;
        u1["role"] = "user";
        QJsonArray c1;
        c1.append(QJsonObject{{"type","text"}, {"text", m_cachedPrefixByKey.value(ctx->cachingKey)}});
        u1["content"] = c1;
        input.append(u1);
    }

    // Second user message: the current question (+ optional image)
    {
        QJsonObject u2;
        u2["role"] = "user";
        QJsonArray c2;
        c2.append(QJsonObject{{"type","text"}, {"text", ctx->question}});

        if (!ctx->imagePngBase64.isEmpty())
        {
            c2.append(QJsonObject{
                {"type","input_image"},
                {"image_data", QString::fromLatin1(ctx->imagePngBase64)},
                {"mime_type", _mimePNG()}
            });
        }

        u2["content"] = c2;
        input.append(u2);
    }

    QJsonObject body;
    body["model"] = ctx->model;
    body["input"] = input;

    // No streaming; no explicit response_format (free text).
    // If you want strict JSON, set a json_schema here exactly like earlier examples.

    QJsonDocument d(body);
    return d.toJson(QJsonDocument::Compact);
}

void OpenAi::_retry(InFlight* ctx)
{
    ctx->attempt++;
    const int backoffMs = 500 * (1 << (ctx->attempt - 1)); // 500, 1000, 2000...
    QTimer::singleShot(backoffMs, this, [this, ctx]{ _send(ctx); });
}

bool OpenAi::_shouldRetry(QNetworkReply* r) const
{
    const int code = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return (code == 429 || (code >= 500 && code < 600));
}

void OpenAi::_finalize(InFlight* ctx, bool ok, const QString& err, const QString& text)
{
    if (ctx->timer) { ctx->timer->deleteLater(); ctx->timer = nullptr; }

    if (!ok)
    {
        ExceptionOpenAiError exception;
        exception.setError(err);
        exception.raise();
    }

    if (ctx->callback)
    {
        QString out = text;
        ctx->callback(out);
    }

    delete ctx;
}

QString OpenAi::_mimePNG()
{
    return "image/png";
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

//*/
