#ifndef OPENAI_H
#define OPENAI_H

#include <QString>
#include <QQueue>
#include <QObject>
#include <QImage>
#include <QNetworkReply>
#include <QNetworkAccessManager>

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
        std::function<void(QString&)> callbackReply,
        const QString& model = "gpt-4.1-mini");

    // With image
    void askQuestion(
        const QString& question,
        const QImage& image,
        const QString& cachingKey,
        std::function<void(QString&)> callbackReply,
        const QString& model = "gpt-4.1-mini");

    // Throughput controls
    void setMaxInFlight(int n);   // e.g. 8
    void setMinSpacingMs(int ms); // e.g. 50..150
    int  maxInFlight() const;
    int  minSpacingMs() const;

    // Timeouts / retries
    int  maxRetries() const;
    void setMaxRetries(int newMaxRetries);
    int  timeoutMs() const;
    void setTimeoutMs(int newTimeoutMs);

    // Prompt-caching control
    void setCachedPrefix(const QString& cachingKey, const QString& prefixText); // freeze a shared prefix
    void clearCachedPrefix(const QString& cachingKey);
    bool hasCachedPrefix(const QString& cachingKey) const;
    QString cachedPrefix(const QString& cachingKey) const;

private:
    explicit OpenAi(QObject* parent = nullptr);
    void _raiseExceptionIfNotInitialized();

    struct InFlight {
        QString id;
        QString model;
        QString cachingKey;
        QString question;
        QByteArray imagePngBase64; // empty if no image
        std::function<void(QString&)> callback;
        int attempt = 0;
        QNetworkReply* reply = nullptr;
        QTimer* timer = nullptr;
    };

    struct Pending {
        QString question;
        QString cachingKey;
        QImage  image;
        QString model;
        std::function<void(QString&)> cb;
    };

    // State
    bool m_initialized = false;
    QString m_openAiKey;
    int m_maxRetries = 3;
    int m_timeoutMs  = 30000;

    int m_maxInFlight = 8;
    int m_minSpacingMs = 100;
    int m_inFlight = 0;
    bool m_dispatchScheduled = false;

    QHash<QString, QString> m_cachedPrefixByKey;
    QQueue<Pending> m_queue;
    QNetworkAccessManager* m_net = nullptr;

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
    void _finalize(InFlight* ctx, bool ok, const QString& err, const QString& text = QString());

    static QString _mimePNG();
    static QByteArray _toPngBase64(const QImage& img);
};

/*
class OpenAi : public QObject
{
    Q_OBJECT
public:
    static const QString KEY_OPEN_AI_API_KEY;
    static OpenAi *instance();
    ~OpenAi();
    bool isInitiatized() const;
    void init(const QString &openAiKey);
    void askQuestion(
            const QString &question,
            const QString &cachingKey,
            std::function<void(QString &)> callbackReply,
            const QString &model = "gpt-4.1-mini");
    void askQuestion(
            const QString &question,
            const QImage &image,
            const QString &cachingKey,
            std::function<void(QString &)> callbackReply,
            const QString &model = "gpt-4.1-mini");

    int maxRetries() const;
    void setMaxRetries(int newMaxRetries);

    int timeoutMs() const;
    void setTimeoutMs(int newTimeoutMs);

private:
    static const QUrl RESPONSES_URL;
    struct InFlight {
        QString id;
        QString model;
        QString cachingKey;
        QString question;
        QByteArray imagePngBase64; // empty if no image
        std::function<void(QString &)> callback;
        int attempt = 0;
        QNetworkReply* reply = nullptr;
        QTimer* timer = nullptr;
    };
    OpenAi(QObject *parent = nullptr);
    bool m_initialized;
    QString m_openAiKey;
    int m_maxRetries;
    int m_timeoutMs;
    void _raiseExceptionIfNotInitialized();
    QNetworkAccessManager* m_net;

    // Cached shared prefixes per cachingKey (what triggers prompt caching)
    QHash<QString, QString> m_cachedPrefixByKey;

    // Helpers
    void _send(InFlight* ctx);
    QByteArray _buildBody(const InFlight* ctx) const;
    void _finalize(InFlight* ctx, bool ok, const QString& err, const QString& text = QString());
    void _retry(InFlight* ctx);
    bool _shouldRetry(QNetworkReply* r) const;

    static QString _mimePNG();
    static QByteArray _toPngBase64(const QImage& img);

};
//*/

#endif // OPENAI_H
