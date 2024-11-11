#include <QDebug>
#include <QFileInfo>

#include "FtpManager.h"

//----------------------------------------
const int FtpManager::N_QUERIES_MAX = 20;
const int FtpManager::N_MSECS_TIMER = 1000;
//----------------------------------------
FtpManager::FtpManager(QObject *parent)
    : QObject(parent)
{
    m_process = nullptr;
    m_nQueries = 0;
    m_nQueriesTotalFinished = 0;
    connect(&m_timer,
            &QTimer::timeout,
            this,
            &FtpManager::_processArgumentsInQueue);
}
//----------------------------------------
FtpManager::~FtpManager()
{
    delete m_process;
}
//----------------------------------------
void FtpManager::uploadFile(
        const QString &host,
        const QString &userName,
        const QString &pwd,
        const QString &filePath,
        int)
{
    QStringList arguments;
    int lastIndexSlash = host.lastIndexOf("/");
    if (lastIndexSlash > 10) {
        arguments << "--ftp-create-dirs";
    }
    arguments << "-u";
    arguments << userName + ":" + pwd;
    arguments << "-T";
    arguments << filePath;
    arguments << host;
    //_createProcessIfNeeded();
    qDebug() << "Uploading file:" << QFileInfo(filePath).absoluteFilePath();
    _startProcessOrPutInQueue(arguments);
}
//----------------------------------------
void FtpManager::uploadFilesOneGroupByOne(
        const QString &host,
        const QString &userName,
        const QString &pwd,
        const QStringList &fileNames,
        const QString &dirPath,
        int)
{
    QStringList arguments;
    int lastIndexSlash = host.lastIndexOf("/");
    if (lastIndexSlash > 10) {
        arguments << "--ftp-create-dirs";
    }
    arguments << "-u";
    arguments << userName + ":" + pwd;
    arguments << "-T";
    QString joinFileNames(fileNames.join(","));
    joinFileNames.insert(0, "{");
    joinFileNames += "}";
    arguments << joinFileNames;
    arguments << host;
    //_createProcessIfNeeded();
    qDebug() << "Uploading file:" << fileNames.size() << " - first file is:" << fileNames[0] << " to " << fileNames.last();
    _addArgumentsToProcessOneByOne(arguments, dirPath);
}
//----------------------------------------
void FtpManager::uploadFileOneByOne(
        const QString &host,
        const QString &userName,
        const QString &pwd,
        const QString &filePath,
        int)
{
    QStringList arguments;
    int lastIndexSlash = host.lastIndexOf("/");
    if (lastIndexSlash > 10) {
        arguments << "--ftp-create-dirs";
    }
    arguments << "-u";
    arguments << userName + ":" + pwd;
    arguments << "-T";
    arguments << filePath;
    arguments << host;
    //_createProcessIfNeeded();
    qDebug() << "Uploading file:" << QFileInfo(filePath).absoluteFilePath();
    _addArgumentsToProcessOneByOne(arguments);
}
//----------------------------------------
void FtpManager::_startProcessOrPutInQueue(
        const QStringList &arguments)
{
    /*
    QString program;
#ifdef Q_OS_LINUX
    program = "curl";
#else
    program = "curl-7.85.0_9-win64-mingw/bin/curl.exe";
#endif
//*/
    if (m_nQueries < N_QUERIES_MAX) {
                /*
                m_process = new QProcess();
        auto process = new QProcess;
        connect(process,
                &QProcess::errorOccurred,
                this,
                &FtpManager::onErrorOccured);
        connect(process,
                &QProcess::finished,
                this,
                &FtpManager::onFinished);
        connect(process,
                &QProcess::finished,
                this,
                [process](){
            process->deleteLater();
        });
        process->start(program, arguments);
        ++m_nQueries;
        ++m_nQueriesTotalFinished;
        //*/
        _runProcess(arguments);
    } else {
        m_argumentsInQueue << arguments;
        if (!m_timer.isActive()) {
            m_timer.start(N_MSECS_TIMER);
        }
    }
}
//----------------------------------------
void FtpManager::_addArgumentsToProcessOneByOne(
        const QStringList &arguments, const QString &dirPath)
{
    _createProcessIfNeeded();
    if (!dirPath.isEmpty()) {
        m_process->setWorkingDirectory(dirPath);
    }
    m_process->waitForFinished();
    //qDebug() << m_process->readAllStandardError();
    qDebug() << m_process->readAllStandardOutput();
#ifdef Q_OS_LINUX
    QString program("curl");
#else
    QString program("curl-7.85.0_9-win64-mingw/bin/curl.exe");
#endif
    m_process->start(program, arguments);
}
//----------------------------------------
void FtpManager::_runProcess(const QStringList &arguments)
{
#ifdef Q_OS_LINUX
    QString program("curl");
#else
    QString program("curl-7.85.0_9-win64-mingw/bin/curl.exe");
#endif
    qDebug() << program << arguments.join(" ");
    auto process = new QProcess;
    connect(process,
            &QProcess::errorOccurred,
            this,
            [this, process](QProcess::ProcessError error){
        QString errorText = process->readAllStandardError();
        emit errorOccured(error, errorText);
    });
    //&FtpManager::onErrorOccured);
    connect(process,
            &QProcess::finished,
            this,
            &FtpManager::onFinished);
    connect(process,
            &QProcess::finished,
            this,
            [process](){
        process->deleteLater();
    });
    process->start(program, arguments);
    ++m_nQueries;
    //++m_nQueriesTotalFinished;
}
//----------------------------------------
void FtpManager::_processArgumentsInQueue()
{
    int nSpots = N_QUERIES_MAX - m_nQueries;
    for (int i=0; i<nSpots; ++i) {
        if (m_argumentsInQueue.size() == 0) {
            m_timer.stop();
            return;
        }
        auto arguments = m_argumentsInQueue.takeFirst();
        _runProcess(arguments);
        //m_process->start(program, arguments);
    }
}
//----------------------------------------
//*
void FtpManager::_createProcessIfNeeded()
{
    if (m_process == nullptr) {
        m_process = new QProcess();
        connect(m_process,
                &QProcess::errorOccurred,
                this,
                &FtpManager::onErrorOccured);
        connect(m_process,
                &QProcess::finished,
                this,
                &FtpManager::onFinished);
    }
}
//*/
//----------------------------------------
void FtpManager::uploadFileIfNew(
        const QString &host,
        const QString &userName,
        const QString &pwd,
        const QString &filePath,
        int)
{
    QStringList arguments;
    arguments << "-u";
    arguments << userName + ":" + pwd;
    arguments << "-T";
    arguments << filePath;
    arguments << "-z";
    arguments << QFileInfo(filePath).fileName();
    arguments << host;
    //_createProcessIfNeeded();
    qDebug() << "Uploading file if new:" << QFileInfo(filePath).absoluteFilePath();
    QString temp = arguments.join(" ");
    _startProcessOrPutInQueue(arguments);
}
//----------------------------------------
void FtpManager::createDirectory(
        const QString &host,
        const QString &userName,
        const QString &pwd,
        const QString &dirName,
        int)
{
    QStringList arguments;
    arguments << "--ftp-create-dirs";
    arguments << "-u";
    arguments << userName + ":" + pwd;
    arguments << host + "/" + dirName;
    //_createProcessIfNeeded();
    qDebug() << "Creating directory:" << dirName;
    _startProcessOrPutInQueue(arguments);
}
//----------------------------------------
void FtpManager::resetNQueries()
{
    m_nQueries = 0;
    m_nQueriesTotalFinished = 0;
}
//----------------------------------------
void FtpManager::onErrorOccured(
        QProcess::ProcessError error)
{
    QString errorText = m_process->readAllStandardError();
    emit errorOccured(error, errorText);
}
//----------------------------------------
void FtpManager::onFinished(
        int,
        QProcess::ExitStatus)
{
    --m_nQueries;
    ++m_nQueriesTotalFinished;
    emit queryFinished(m_nQueriesTotalFinished);
}
//----------------------------------------
//----------------------------------------
