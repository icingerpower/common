#ifndef OPENAI_H
#define OPENAI_H

#include <QString>
#include <QQueue>
#include <QObject>
#include <QImage>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QElapsedTimer>

#include <QHash>
#include <QQueue>
#include <QUrl>
#include <functional>

class QTimer;
class QThread;

class OpenAi : public QObject
{
    Q_OBJECT
public:
    static const QString KEY_OPEN_AI_API_KEY;
    static const QUrl RESPONSES_URL;

    static OpenAi* instance();
    ~OpenAi();

    bool isInitiatized() const;
    void init(const QString& openAiKey);

    // Text only
    void askQuestion(
        const QString& question,
        const QString& cachingKey,
        int nMaxRetryOnReplyFailed,
        const QString& model,
        std::function<bool(const QString& json)> callbackTryProcessReply,  // return true = accepted
        std::function<void(const QString& json)> callbackReplySuccess,     // executed on caller thread
        std::function<void(const QString& json)> callbackReplyFailure);    // executed on caller thread, gets last reply or error json

    // With image
    void askQuestion(
        const QString& question,
        const QImage& image,
        const QString& cachingKey,
        int nMaxRetryOnReplyFailed,
        const QString& model,
        std::function<bool(const QString& json)> callbackTryProcessReply,
        std::function<void(const QString& json)> callbackReplySuccess,
        std::function<void(const QString& json)> callbackReplyFailure);

    // Throughput controls
    void setMaxInFlight(int n);   // default 8
    void setMinSpacingMs(int ms); // default 100
    int  maxInFlight() const;
    int  minSpacingMs() const;

    // Timeouts / retries (transport)
    int  maxRetries() const;             // default 3
    void setMaxRetries(int newMaxRetries);
    int  timeoutMs() const;              // default 30000
    void setTimeoutMs(int newTimeoutMs);

    // Prompt-caching control
    void setCachedPrefix(const QString& cachingKey, const QString& prefixText);
    void clearCachedPrefix(const QString& cachingKey);
    bool hasCachedPrefix(const QString& cachingKey) const;
    QString cachedPrefix(const QString& cachingKey) const;

    QNetworkAccessManager* _netForAttempt(int attempt);

private:
    explicit OpenAi(QObject* parent = nullptr);
    void _raiseExceptionIfNotInitialized();

    struct InFlight {
        QString id, model, cachingKey, question;
        QByteArray imagePngBase64; // empty if no image
        std::function<bool(const QString&)> callbackTry;
        std::function<void(const QString&)> callbackSuccess;
        std::function<void(const QString&)> callbackFailure;
        int semanticRetriesLeft = 0; // retry on "bad" model reply
        int attempt = 0;             // transport retries
        QNetworkReply* reply = nullptr;
        QTimer* timer = nullptr;
        QThread* callerThread = nullptr; // to marshal callbacks
        QObject* proxy = nullptr;        // lives in callerThread
        QByteArray lastRaw;              // last raw JSON/text for failure callback
    };

    struct Pending {
        QString question, cachingKey, model;
        QImage image;
        std::function<bool(const QString&)> callbackTry;
        std::function<void(const QString&)> callbackSuccess;
        std::function<void(const QString&)> callbackFailure;
        int semanticRetries = 0;
    };

    // State
    bool m_initialized = false;
    QString m_openAiKey;
    QElapsedTimer m_lastSend;
    bool m_lastSendInit = false;
    void _sendWithPacing(InFlight* ctx, int minDelayMs); // NEW
    int m_maxRetries = 5;
    int m_timeoutMs  = 100000;
    int m_maxInFlight = 1;
    int m_minSpacingMs = 1000;
    int m_inFlight = 0;
    bool m_dispatchScheduled = false;

    QHash<QString, QString> m_cachedPrefixByKey;
    QQueue<Pending> m_queue;
    //QNetworkAccessManager* m_net = nullptr;
    QHash<int, QNetworkAccessManager *> m_attempt_networkAccessManager;

    // Queueing
    void _enqueue(const Pending& p);
    void _scheduleDispatch(int delayMs = 0);
    void _dispatchOne();

    // Network
    void _send(InFlight* ctx);
    QByteArray _buildBody(const InFlight* ctx) const;

    // Helpers
    void _retry(InFlight* ctx, const QString& retryAfterHeader = QString());
    bool _shouldRetry(QNetworkReply* r) const;
    void _finalize(InFlight* ctx, bool ok, const QString& err);

    // Callback marshaling
    void _postSuccess(InFlight* ctx, const QString& text);
    void _postFailure(InFlight* ctx, const QString& jsonOrErr);

    static QByteArray _toPngBase64(const QImage& img);
    static QString _imageDataUrlFromBase64Png(const QByteArray& b64png);
};

#endif // OPENAI_H
