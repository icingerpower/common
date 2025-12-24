#ifndef OPENAI2_H
#define OPENAI2_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QQueue>
#include <functional>


class OpenAi2 : public QObject
{
    Q_OBJECT
    friend class TestOpenAi2;
public:
    static const QString KEY_OPEN_AI_API_KEY;
    static const QUrl RESPONSES_URL;
    
    struct DebugState {
        bool initialized;
        int maxQueriesSameTime;
        int inFlightText;
        int inFlightImage;
        bool pumping;
        qint64 blockedUntilMs;
        int consecutiveHardFailures;
        int pendingTextSize;
        int pendingImageSize;
    };

    static OpenAi2* instance();
    struct Step{
        QString id;
        QString name;
        QString cachingKey;
        QList<QString> imagePaths;
        QString gptModel = "gpt-5-mini";
        QString gptModelAfterHalfFailure = "gpt-5.2"; // Once the number of failure = maxRetries / 2, we use a better model

        std::function<QString(int nAttempts)> getPrompt;
        std::function<void(const QString &gptReply)> apply;
        std::function<bool(const QString &gptReply, const QString &lastWhy)> validate; // Last why help to know the kind of error
        int maxRetries = 10;
        QString getGptModel(int attempt) const;
    };
    void init(const QString &apiKey
            , int maxQueriesSameTime = 5
            , int maxQueriesImageSameTime = 1
            , int timeoutMsBetweenImageQueries = 30000);

    // Test Hooks
    void setTransportForTests(std::function<void(const QString&, const QString&, const QList<QString>&, std::function<void(QString)>, std::function<void(QString)>)> transport);
    void resetForTests();
    DebugState getDebugStateForTests() const;

    struct StepMultipleAsk : Step{ // Prompt is asked several time and a function will ask the best reply
        int neededReplies = 3; // For instance 3 valid replies are needed before a best reply will be choosen
        std::function<void(std::function<QString(const QList<QString> &gptReplies)>)> getBestReply;
    };

    struct StepMultipleAskAi : Step{ // Prompt is asked several time and the AI will select the best reply
        int neededReplies = 3; // For instance 3 valid replies are needed
        std::function<QString(int nAttempts, const QList<QString> &gptValidReplies)> getPromptGetBestReply;
        std::function<bool(const QString &gptReply, const QString &lastWhy)> validateBestReply;
    };


    void askGpt(
            const QList<QSharedPointer<Step>> &stepsInQueue
            , const QString &model
            );
    void askGptMultipleTime(
            const QList<QSharedPointer<StepMultipleAsk>> &stepsInQueue
            , const QString &model
            );
    void askGptMultipleTimeAi(
            const QList<QSharedPointer<StepMultipleAskAi>> &stepsInQueue
            , const QString &model
            );

    void askGptBatch(
            const QList<QSharedPointer<Step>> &stepsIndependants
            , const QString &model
            , const QString &progressCsvFilePath // using step ID and this csv file content, can now if batch ID needs to be asked for the first time or if we need to retriev batch results
            , int nQueriesLeftForNonBatch // When failure, we re-run queries. If few left due to many failure, we send query normally instead of by batch
            );
    void askGptBatchMultipleTime(
            const QList<QSharedPointer<StepMultipleAsk>> &stepsIndependants
            , const QString &model
            , const QString &progressCsvFilePath
            , int nQueriesLeftForNonBatch
            );
    void askGptBatchMultipleTimeAi(
            const QList<QSharedPointer<StepMultipleAskAi>> &stepsIndependants
            , const QString &model
            , const QString &progressCsvFilePath
            , int nQueriesLeftForNonBatch
            );

private:
    OpenAi2(QObject *parent = nullptr);
    // Executes a queue sequentially (or with concurrency later)
    void _runQueue(
        const QList<QSharedPointer<Step>>& steps,
        std::function<void()> onAllSuccess,
        std::function<void(QString)> onAllFailure);

    // Runs one step with retries, caching, model upgrade, etc.
    void _runStepWithRetries(
        const QSharedPointer<Step>& step,
        std::function<void(QString raw)> onStepSuccess,
        std::function<void(QString err)> onStepFailure);

    // Low-level single call to OpenAI Responses API
    void _callResponses(
        const QString& model,
        const QString& prompt,
        const QList<QString>& imagePaths,
        std::function<void(QString raw)> onOk,
        std::function<void(QString err)> onErr);

    // Cache helpers
    bool _tryLoadCache(const Step& step, QString* rawOut) const;
    void _storeCache(const Step& step, const QString& raw) const;

    // Model selection
    QString _selectModelForAttempt(const Step& step, int attempt) const;

    // Optional: throttle/scheduler hooks (based on m_maxQueriesSameTime / images)
    void _schedule(std::function<void(std::function<void()>)> startRequest);
    void _runStepCollectN(
        const QSharedPointer<Step>& step,
        int neededReplies,
        std::function<QString(const QList<QString>& valids)> chooseBest,
        std::function<void(QString best)> onBest,
        std::function<void(QString err)> onFail);

    void _runStepCollectNThenAskBestAI(
        const QSharedPointer<Step>& step,
        int neededReplies,
        std::function<QString(int attempt, const QList<QString>& valids)> getPromptGetBestReply,
        std::function<bool(const QString&)> validateBest,
        std::function<void(QString best)> onBest,
        std::function<void(QString err)> onFail);

    void _runBatch(
        const QList<QSharedPointer<Step>>& steps,
        const QString& model,
        const QString& progressCsv,
        int nFallbackNonBatch,
        std::function<void()> onAllSuccess,
        std::function<void(QString)> onAllFailure);

    QByteArray _buildBatchJsonl(const QList<QSharedPointer<Step>>& steps, const QString& model) const;
    QHash<QString, QString> _parseBatchOutput(const QByteArray& jsonlOut) const; // id -> raw
    
    void _onInternalCallSuccess();
    void _onInternalCallError(const QString& err);

    QNetworkAccessManager m_networkAccessManager;
    int m_maxQueriesSameTime;
    int m_maxQueriesImageSameTime;
    int m_timeoutMsBetweenImageQueries; // To avoid being blocked as queries with image easily reach quota
    bool m_initialized;
    std::function<void(const QString& model, const QString& prompt, const QList<QString>& imagePaths, std::function<void(QString raw)> onOk, std::function<void(QString err)> onErr)> m_transport;
    void _callResponses_Real(
        const QString& model,
        const QString& prompt,
        const QList<QString>& imagePaths,
        std::function<void(QString raw)> onOk,
        std::function<void(QString err)> onErr);

    QString m_openAiKey;
    QQueue<std::function<void(std::function<void()>)>> m_pendingText;
    QQueue<std::function<void()>> m_pendingImage;
    int m_inFlightText;
    int m_inFlightImage;
    bool m_pumping;
    qint64 m_blockedUntilMs;
    int m_consecutiveHardFailures;

};

#endif // OPENAI2_H
