#include <QThread>
#include <QDebug>

#include "ExceptionOpenAiNotInitialized.h"
#include "ExceptionOpenAiError.h"

#include "OpenAi.h"

//----------------------------------------
const QString OpenAi::KEY_SET_LAST_TYPE = "open-ai-tab-last-article-type";
const QString OpenAi::KEY_LAST_WORKING_DIR = "open-ai-python-working-dir";
const QString OpenAi::KEY_TEST_DATA_WORKING_DIR = "open-ai-test-data-dir";
const QString OpenAi::KEY_OPEN_AI_API_KEY = "open-ai-api-key";
//----------------------------------------
const int OpenAi::N_PROCESSES = 5;
//----------------------------------------
OpenAi::OpenAi()
{
    m_initialized = false;
    m_nProcessRead = 0;
    //m_process = new QProcess();
}
//----------------------------------------
OpenAi::~OpenAi()
{
    //m_process->deleteLater();
    for (auto it = m_processes.begin();
         it != m_processes.end(); ++it) {
        (*it)->deleteLater();
    }
}
//----------------------------------------
OpenAi *OpenAi::instance()
{
    static OpenAi instance;
    return &instance;
}
//----------------------------------------
bool OpenAi::isInitiatized() const
{
    return m_initialized;
}
//----------------------------------------
void OpenAi::init(
        const QString &dirWithEnv, const QString &openAiKey)
{
    m_dirWithEnv = dirWithEnv;
    m_openAiKey = openAiKey;
    if (isInitiatized()) {
        for (auto it = m_processes.begin();
             it != m_processes.end(); ++it) {
            (*it)->deleteLater();
        }
        m_processes.clear();
        //m_process->deleteLater();
        //m_process = new QProcess();
    }
    for (int i=0; i<N_PROCESSES; ++i) {
        auto process = new QProcess();
        process->setWorkingDirectory(dirWithEnv);
        process->start("bash", QStringList() << "-c" << ". venv/bin/activate && python3 -i");
        m_processes << process;
    }
    for (auto it = m_processes.begin();
         it != m_processes.end(); ++it) {
        auto process = *it;
        process->waitForStarted();
        QByteArray commands(
                    "# -*- coding: utf-8 -*-\n"
                    "import openai\n"
                    "openai.api_key = \"");
        commands += openAiKey.toUtf8();
        commands += "\"\n";
        process->write(commands);
    }
    m_processes[0]->waitForBytesWritten();
    m_processes[0]->waitForReadyRead();
    qDebug() << m_processes[0]->readAll();
    /*
    for (auto it = m_processes.begin();
         it != m_processes.end(); ++it) {
        auto process = *it;
        process->waitForBytesWritten();
        process->waitForReadyRead();
        qDebug() << process->readAll();
    }
    //*/
    m_nProcessRead = N_PROCESSES;
    /*
    m_process->setWorkingDirectory(dirWithEnv);
    m_process->start("bash", QStringList() << "-c" << ". venv/bin/activate && python3 -i");
    m_process->waitForStarted();
    QByteArray commands(
                "# -*- coding: utf-8 -*-\n"
                "import openai\n"
                "openai.api_key = \"");
    commands += openAiKey.toUtf8();
    commands += "\"\n";
    m_process->write(commands);
    m_process->waitForBytesWritten();
    m_process->waitForReadyRead();
    qDebug() << m_process->readAll();
    //*/
    m_initialized = true;
}
//----------------------------------------
void OpenAi::_addAndInitProcess()
{
    auto process = new QProcess();
    process->setWorkingDirectory(m_dirWithEnv);
        /*
    process->connect(
                process,
                &QProcess::started,
                process,
                [process, this](){
        QByteArray commands(
                    "# -*- coding: utf-8 -*-\n"
                    "import openai\n"
                    "openai.api_key = \"");
        commands += m_openAiKey.toUtf8();
        commands += "\"\n";
        //process->connect(
                    //process,
                    //&QProcess::bytesWritten,
                    //process,
                    //[this](){
            //++m_nProcessRead;
        //});
        process->write(commands);
        ++m_nProcessRead;
    });
    //*/
    process->start("bash", QStringList() << "-c" << ". venv/bin/activate && python3 -i");
    QByteArray commands(
                "# -*- coding: utf-8 -*-\n"
                "import openai\n"
                "openai.api_key = \"");
    commands += m_openAiKey.toUtf8();
    commands += "\"\n";
    process->write(commands);
    m_processes << process;
}
//----------------------------------------
void OpenAi::_waitProcessToBeReadyForOpenAi(QProcess *process)
{
    process->waitForReadyRead();
    process->readAll();
}
//----------------------------------------
void OpenAi::_terminateProcess(QProcess *process)
{
    process->write("exit()\n");
    process->close();
    process->deleteLater();
}
//----------------------------------------
void OpenAi::askQuestion(
        const QString &question,
        const QList<QPair<QString, QString>> &previousTalk,
        std::function<void (QString &)> callbackReply,
        const QString &model)
{
    QString reply = askQuestion(question, previousTalk, model);
    callbackReply(reply);
}
//----------------------------------------
QString OpenAi::askQuestion(
        const QString &question,
        const QList<QPair<QString, QString>> &previousTalk,
        const QString &model)
{
    if (!m_initialized) {
        ExceptionOpenAiNotInitialized exception;
        exception.raise();
    }
    QString questionAI("[{'role': 'system', 'content': 'You are a blog writer assistant.'},");
    for (auto itUserToIa = previousTalk.begin();
         itUserToIa != previousTalk.end(); ++itUserToIa) {
        if (!itUserToIa->first.isEmpty()) {
            questionAI += "{'role': 'user', 'content': '";
            questionAI += QString(itUserToIa->first).replace("'", "’");
            questionAI += "'},";
        }
        questionAI += "{'role': 'assistant', 'content': '";
        questionAI += QString(itUserToIa->second).replace("'", "’");
        questionAI += "'},";
    }
    questionAI += "{'role': 'user', 'content': '";
    questionAI += QString(question).replace("'", "’");
    questionAI += "'}]";
    auto process = m_processes.takeFirst();
    //--m_nProcessRead;
    //while (m_nProcessRead < 0) {
        //QThread::sleep(1);
    //}
    _addAndInitProcess();
    _waitProcessToBeReadyForOpenAi(process);
    qDebug() << "------\nPREVIOUS read all:" << process->readAll();
    qDebug() << "QUESTION:" << question;
    if (previousTalk.size() > 0) {
        //qDebug() << questionAI;
        int TEMP=10;++TEMP;
    }
    QString reply;
    QString error;
    QByteArray commands("response = openai.ChatCompletion.create(model='gpt-3.5-turbo', messages=");
    commands += questionAI.toUtf8();
    commands += ");print(response['choices'][0]['message']['content'])\n";
    bool commandSuccess = false;
    int attemptCount=0;

    do {
        process->write(commands);
        process->waitForBytesWritten();
        process->waitForReadyRead();
        QThread::msleep(100);
        reply = process->readAllStandardOutput().trimmed();
        for (int i=0; i<20;++i) {
            if (reply.trimmed().isEmpty()) {
                QThread::msleep(1000);
                reply = process->readAllStandardOutput().trimmed();
            }
        }
        if (reply.trimmed().isEmpty() && attemptCount == 1) {
            QThread::msleep(1000 * 60 * 2);
        } else if (reply.trimmed().isEmpty() && attemptCount == 3) {
            QThread::msleep(1000 * 60 * 10);
        }
        //qDebug() << reply;
        QString newError =  process->readAllStandardError();
        qWarning() << "new error:" << newError;
        error += newError;
        // TODO if newError is failed to establish a connection, wait for a while
        ++attemptCount;
        static QString lastReply = "azopeiuei";
        int maxAttemp = 11;
        if (attemptCount == maxAttemp) {
            qDebug() << "11 FAILURES => stopping asking question";
            qDebug() << "COMMAND failed:" << commands;
            break;
        }
        if (newError.contains("Connection refused")
                || newError.contains("RateLimitError")
                || newError.contains("ConnectionResetError")
                || newError.contains("ServiceUnavailableError")
                || newError.contains("NewConnectionError")
                || newError.contains("openai.error.APIConnectionError")
                || newError.contains("openai.error.APIError")
                || newError.contains("HTTPSConnectionPool")) {
            qDebug() << "SLEEPING a few minutes as API overloaded";
            --attemptCount;
            QThread::sleep(600);
            continue;
        } else if (newError.contains("stdin")) {
            qDebug() << "COMMAND failed:" << commands;
            break;
        }

        commandSuccess = !reply.trimmed().isEmpty();
        if (commandSuccess && !lastReply.isEmpty()) {
            commandSuccess = !reply.contains(lastReply);
        }
        if (commandSuccess) {
            if (!model.contains("gpt-4")) {
                commandSuccess = !reply.contains("[")
                        && !reply.contains("]");
            } else {
                reply.replace("[", "(");
                reply.replace("]", ")");
            }
        }
        if ((!commandSuccess || reply.isEmpty())) {
            unsigned long msToSleep = 10 * attemptCount;
            if (attemptCount > 4) {
                qDebug() << "failure number " << attemptCount;
                qDebug() << "Using a new process due to many failure";
                _terminateProcess(process);
                process = m_processes.takeFirst();
                _addAndInitProcess();
                if (attemptCount > 8) {
                    msToSleep = 600;
                }
            }
            QThread::sleep(msToSleep);
        }
            /*
        if (!commandSuccess) {
            qDebug() << "failure number " << attemptCount;
            if (attemptCount > 6) {
                qDebug() << "Using a new process due to many failure";
                _terminateProcess(process);
                process = m_processes.takeFirst();
                _addAndInitProcess();
            }
            if (attemptCount > 8) {
                QThread::sleep(600);
            } else {
                QThread::sleep(10 * attemptCount);
            }
        } else if (reply.isEmpty() && attemptCount > 6) {
            _terminateProcess(process);
            process = m_processes.takeFirst();
            _addAndInitProcess();
        }
            //*/
        lastReply = reply;

    } while (!commandSuccess);
    qDebug() << "REPLY:" << reply;
    qDebug() << "ERROR:" << error;
    _terminateProcess(process);
    //*/
    if (commandSuccess) {
    } else if (error.contains("SyntaxError")
            || error.contains("Traceback")) {
        ExceptionOpenAiError exception;
        exception.setError(QString("Python code error: ") + error);
        exception.raise();
    } else if (reply.isEmpty()) {
        ExceptionOpenAiError exception;
        exception.setError(QString("Empty reply from Open Ai for question: ") + question);
        exception.raise();
    }
    reply.replace("\r\n ", "\n");
    reply.replace("\r", "\n");
    while(reply.contains("\n ")) {
        reply.replace("\n ", "\n");
    }
    while(reply.contains(" \n")) {
        reply.replace(" \n", "\n");
    }
    while(reply.contains("\n\n")) {
        reply.replace("\n\n", "\n");
    }
    return reply.trimmed();
}
//----------------------------------------
