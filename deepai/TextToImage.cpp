#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QImage>
//#include <QNetworkAccessManager>
//#include <QNetworkReply>
//#include <QNetworkRequest>

#include "TextToImage.h"

//----------------------------------------
int TextToImage::DEFAULT_WIDTH = 800;
int TextToImage::DEFAULT_HEIGHT = 800;
QSize TextToImage::DEFAULT_SIZE
= QSize(TextToImage::DEFAULT_WIDTH, TextToImage::DEFAULT_HEIGHT);
//----------------------------------------
TextToImage::TextToImage()
{
}
//----------------------------------------
void TextToImage::retrieveImage(
        const QString &text,
        std::function<void (const QList<QImage> &)> callbackSuccess,
        std::function<void (const QString &)> callbackError,
        const QString &apiKey,
        const QString &proxyHost,
        const QString &proxyPort,
        const QString &proxyUserName,
        const QString &proxyPassword,
        const QString &proxyHostSecond,
        const QString &proxyPortSecond,
        const QString &proxyUserNameSecond,
        const QString &proxyPasswordSecond) //TODO use from IA
{
    QString program;
    QStringList arguments;
#ifdef Q_OS_LINUX
    program = "curl";
#else
    program = "curl-7.85.0_9-win64-mingw/bin/curl.exe";
#endif
    arguments << "-k";
    if (!proxyHost.isEmpty()) {
        arguments << "-x";
        if (proxyPort.isEmpty()) {
            arguments << proxyHost;
        } else {
            arguments << proxyHost + ":" + proxyPort;
        }
    }
    if (!proxyUserName.isEmpty() && !proxyPassword.isEmpty()) {
        arguments << "-U";
        arguments << proxyUserName + ":" + proxyPassword;
    }
    arguments << "-F";
    arguments << "text=" + QString(text).replace(" ", "%20");
    arguments << "-H";
    //arguments << "78a1f59c-3973-4eb0-92d7-9df64b79eaa5"; // REAL one paid
    //arguments << "api-key:quickstart-QUdJIGlzIGNvbWluZy4uLi4K"; // DEMO
    //arguments << "api-key:78a1f59c-3973-4eb0-92d7-9df64b79eaa5"; // OLD
    arguments << "api-key:" + apiKey;
    arguments << "https://api.deepai.org/api/text2img";
    QProcess *process = new QProcess();
    //*
    process->connect(process,
            &QProcess::errorOccurred,
            process,
                     [process, callbackError](QProcess::ProcessError error){
        static QHash<QProcess::ProcessError, QString> errorCodes
                = [](){
            QHash<QProcess::ProcessError, QString> _errorCodes;
            _errorCodes[QProcess::FailedToStart] = QObject::tr("Failed to start");
            _errorCodes[QProcess::Crashed] = QObject::tr("Crashed");
            _errorCodes[QProcess::Timedout] = QObject::tr("Timout");
            _errorCodes[QProcess::WriteError] = QObject::tr("Write error");
            _errorCodes[QProcess::ReadError] = QObject::tr("Read error");
            _errorCodes[QProcess::UnknownError] = QObject::tr("Unknown error");
            return _errorCodes;
        }();

        QString errorText = process->readAllStandardError();
        callbackError(errorCodes[error] + " - " + errorText);
        process->deleteLater();
    });
    //*/
    process->connect(process,
                     &QProcess::finished,
                     process,
                     [process,
                     callbackError,
                     callbackSuccess,
                     program,
                     proxyHostSecond,
                     proxyPortSecond,
                     proxyUserNameSecond,
                     proxyPasswordSecond](
                     int exitCode, QProcess::ExitStatus exitStatus){
        if (exitStatus == QProcess::NormalExit) {
            QString errorText = process->readAllStandardError();
            QString outputText = process->readAllStandardOutput();
            if (outputText.contains(".jpg")) {
                QStringList elements = outputText.split("output_url\": \"");
                QString url = elements.last().split("\"")[0];
                static QString fileExt = QFileInfo(url).fileName().split(".").last();
                QProcess *process2 = new QProcess();
                QStringList arguments2;

                if (!proxyHostSecond.isEmpty()) {
                    arguments2 << "-x";
                    QString hostPort = proxyHostSecond;
                    if (!proxyPortSecond.isEmpty()) {
                        hostPort += ":";
                        hostPort += proxyPortSecond;
                    }
                    arguments2 << hostPort;
                    if (!proxyUserNameSecond.isEmpty()) {
                        arguments2 << "-U";
                        QString userPwd = proxyUserNameSecond;
                        if (!proxyPasswordSecond.isEmpty()) {
                            userPwd += ":";
                            userPwd += proxyPasswordSecond;
                        }
                        arguments2 << userPwd;
                    }
                }
                arguments2 << url;
                //arguments2 << ">";
                //QString tempImageFileName = QDateTime::currentDateTime().toString(
                            //"yyyyMMdd_hhmmss_zzz") + "." + fileExt;
                //arguments2 << tempImageFileName;

                process2->connect(process2,
                                  &QProcess::finished,
                                  process2,
                                  [process2, callbackError, callbackSuccess, program](
                                  int exitCode, QProcess::ExitStatus exitStatus){
                    if (exitStatus == QProcess::NormalExit) {
                        QString errorText2 = process2->readAllStandardError();
                        //QString outputText2 = process2->readAllStandardOutput();
                        //if (true) {
                            QImage image;
                            image.loadFromData(process2->readAllStandardOutput());
                            if (image.isNull()) {
                                int TEMP=10;++TEMP;
                            }
                            int side = 512;
                            QList<QImage> images;
                            images << image.copy(0, 0, side, side);
                            images << image.copy(side, 0, side, side);
                            images << image.copy(0, side, side, side);
                            images << image.copy(side, side, side, side);
                            //QFile::remove(tempImageFileName);
                            callbackSuccess(images);
                        //} else {
                            //callbackError(errorText2 + "\n" + outputText2);
                        //}
                    } else {
                        callbackError("Image retrieving crashed");
                    }
                    process2->deleteLater();
                });
                process2->start(program, arguments2);

                /*
                QNetworkAccessManager *networkAccess = new QNetworkAccessManager();
                networkAccess->connect(
                            networkAccess,
                            &QNetworkAccessManager::finished,
                            networkAccess,
                            [](QNetworkReply *reply){
                    QPixmap pixmap;
                    pixmap.loadFromData(reply->readAll());
                    pixmap.save("/home/cedric/testqt.jpg");
                });
                const QUrl qurl = QUrl(url);
                QNetworkRequest request(url);
                networkAccess->get(request);
                //*/
            } else {
                callbackError(errorText + "\n" + outputText);
            }
        } else {
            callbackError("Image retrieving crashed");
        }
        //*
        /*
        QStringList closeArguments = {"-H", "Connection: close", "https://api.deepai.org/api/text2img"};
        process->start(program, closeArguments);
        QTimer::singleShot(10000,
                           process,
                           [process]{
            process->deleteLater();
        });
        //*/
    });
    process->start(program, arguments);
}
//----------------------------------------
