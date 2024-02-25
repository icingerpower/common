#ifndef OPENAI_H
#define OPENAI_H

#include <QString>
#include <QProcess>

class OpenAi
{
public:
    static const QString KEY_SET_LAST_TYPE;
    static const QString KEY_LAST_WORKING_DIR;
    static const QString KEY_TEST_DATA_WORKING_DIR;
    static const QString KEY_OPEN_AI_API_KEY;
    static const int N_PROCESSES;
    static OpenAi *instance();
    ~OpenAi();
    bool isInitiatized() const;
    void init(const QString &dirWithEnv,
              const QString &openAiKey);
    void askQuestion(
            const QString &question,
            const QList<QPair<QString, QString>> &previousTalk,
            std::function<void(QString &)> callbackReply,
            const QString &model = "gpt-3.5-turbo");
    QString askQuestion(
            const QString &question,
            const QList<QPair<QString, QString>> &previousTalk = QList<QPair<QString, QString>>(),
            const QString &model = "gpt-3.5-turbo");

private:
    OpenAi();
    bool m_initialized;
    //QProcess *m_process;
    QList<QProcess *> m_processes;
    QString m_dirWithEnv;
    QString m_openAiKey;
    void _addAndInitProcess();
    void _waitProcessToBeReadyForOpenAi(QProcess *process);
    int m_nProcessRead;
    void _terminateProcess(QProcess *process);
};

#endif // OPENAI_H
