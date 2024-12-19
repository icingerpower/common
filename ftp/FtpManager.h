#ifndef FTPMANAGER_H
#define FTPMANAGER_H

#include <QString>
#include <QTimer>
#include <QProcess>


class FtpManager : public QObject
{
    Q_OBJECT
public:
    static const int N_QUERIES_MAX;
    static const int N_MSECS_TIMER;
    explicit FtpManager(QObject *parent = nullptr);
    ~FtpManager();
    void uploadFile( // TODO return "" if success
            const QString &host,
            const QString &userName,
            const QString &pwd,
            const QString &filePath,
            int port = -1);
    void uploadFilesOneGroupByOne(// TODO return "" if success
            const QString &host,
            const QString &userName,
            const QString &pwd,
            const QStringList &fileNames,
            const QString &dirPath,
            int port = -1);
    void uploadFileOneByOne( // TODO return "" if success
            const QString &host,
            const QString &userName,
            const QString &pwd,
            const QString &filePath,
            int port = -1);
    void uploadFileIfNew(
            const QString &host,
            const QString &userName,
            const QString &pwd,
            const QString &filePath,
            int port = -1);
    void createDirectory(const QString &host,
            const QString &userName,
            const QString &pwd,
            const QString &dirName,
            int port = -1);

    void resetNQueries();


signals:
    void errorOccured(QProcess::ProcessError error,
                      const QString &errorText);
    void queryFinished(int nQueries);

private slots:
    void onErrorOccured(QProcess::ProcessError error);
    void onFinished(int exitCode,
                    QProcess::ExitStatus exitStatus);

    void _processArgumentsInQueue();


private:
    QProcess *m_process;
    int m_nQueries;
    int m_nQueriesTotalFinished;
    void _createProcessIfNeeded();
    void _startProcessOrPutInQueue(const QStringList &arguments);
    void _addArgumentsToProcessOneByOne(
            const QStringList &arguments,
            const QString &dirPath = QString());
    QList<QStringList> m_argumentsInQueue;
    QTimer m_timer;
    void _runProcess(const QStringList &arguments);
    void _runExceptionIfCurlNotFound();
    QString _getCurlProgram() const;
};

#endif // FTPMANAGER_H
