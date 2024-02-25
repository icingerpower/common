#include <QDebug>

#include "ExceptionFtpSettings.h"

#include "ScpManager.h"


//----------------------------------------
QString ScpManager::KEY_RSA;
//rsync -avz --ignore-existing -e "ssh -i /home/cedric/.ssh/id_rsa.pub" . root@162.254.38.191:/home/cryoqitd/rentalthe.com
//----------------------------------------
ScpManager::ScpManager(QObject *parent)
    : QObject(parent)
{

}
//----------------------------------------
QString ScpManager::getRsaKeyCommand()
{
    return "ssh-keygen -t rsa";
}
//----------------------------------------
bool ScpManager::uploadDir(const QString &dirFrom,
        const QString &host,
        const QString &hostDirPath,
        const QString &userName,
        const QString &pwd,
        const QString &rsaFilePath,
        bool ignoreExisting,
        bool useSshPass)
{
    if (rsaFilePath.isEmpty()) {
        ExceptionFtpSettings exception;
        exception.setError(tr("Le fichier RSA n’est pas renseigné"));
        exception.raise();
    }
    QString program = useSshPass ? "sshpass" : "rsync";
    QStringList arguments;
    //rsync -avz --ignore-existing -e "ssh -i /home/cedric/.ssh/id_rsa.pub" . root@162.254.38.191:/home/cryoqitd/rentalthe.com
    if (useSshPass) {
        arguments << "-p"; // for sshpass
        arguments << pwd; // for sshpass
        arguments << "rsync"; // for sshpass
    } else {
        Q_ASSERT(false); //In that case I should write the password in prompt when password is asked
    }
    arguments << "-avz";
    if (ignoreExisting) {
        arguments << "--ignore-existing";
    }
    arguments << "-e";
    QString sshRsaCommand = "ssh -i ";
    sshRsaCommand += rsaFilePath;
    arguments << sshRsaCommand;
    arguments << dirFrom + (dirFrom.endsWith("/") ? "*" : "/*");
    QString connectionInfo = userName;
    connectionInfo += "@";
    connectionInfo += host;
    connectionInfo += ":";
    connectionInfo += hostDirPath;
    arguments << connectionInfo;
    auto exitStatus = QProcess::CrashExit;
    int exitCode = 1;
    int nAttempts = 0;
    while (exitStatus != QProcess::NormalExit && exitCode != 0) {
        if (nAttempts == 10) {
            break;
        }
        QProcess process;
        QString program("sshpass");
        qDebug() << program << arguments.join(" ");
        process.start(program, arguments);
        while(!process.waitForFinished(5000)) {
            qDebug() << process.readAllStandardOutput();
        }
        exitStatus = process.exitStatus();
        exitCode = process.exitCode();
        qDebug() << process.readAllStandardOutput();
        qDebug() << "ERROR FCP ??";
        qWarning() << process.readAllStandardError();
        break;
    }
    bool success = exitStatus == QProcess::NormalExit && exitCode == 0;
    return success;
}
//----------------------------------------
QString ScpManager::getCommand(
        const QString &dirFrom,
        const QString &host,
        const QString &hostDirPath,
        const QString &userName,
        const QString &pwd,
        const QString &rsaFilePath,
        bool ignoreExisting,
        bool useSshPass) const
{
    QString program = useSshPass ? "sshpass" : "rsync";
    QStringList arguments;
    if (useSshPass) {
        arguments << "-p"; // for sshpass
        arguments << pwd; // for sshpass
        arguments << "rsync"; // for sshpass
    }
    arguments << "-avz";
    if (ignoreExisting) {
        arguments << "--ignore-existing";
    }
    arguments << "-e";
    QString sshRsaCommand = "\"ssh -i ";
    sshRsaCommand += rsaFilePath;
    sshRsaCommand += "\"";
    arguments << sshRsaCommand;
    arguments << dirFrom + (dirFrom.endsWith("/") ? "*" : "/*");
    QString connectionInfo = userName;
    connectionInfo += "@";
    connectionInfo += host;
    connectionInfo += ":";
    connectionInfo += hostDirPath;
    arguments << connectionInfo;
    program += " ";
    program += arguments.join(" ");
    return program;
}
//----------------------------------------
