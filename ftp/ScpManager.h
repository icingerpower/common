#ifndef SCPMANAGER_H
#define SCPMANAGER_H

#include <QProcess>

//rsync -avz --ignore-existing -e "ssh -i /home/cedric/.ssh/id_rsa.pub" . root@162.254.38.191:/home/cryoqitd/rentalthe.com
// scp   -p -i /home/cedric/.ssh/id_rsa.pub  -o StrictHostKeyChecking=no ./.htaccess root@162.254.38.191:/home/cryoqitd/rentalthe.com/
class ScpManager : public QObject
{
    Q_OBJECT
public:
    explicit ScpManager(QObject *parent = nullptr);
    static QString KEY_RSA;
    static QString getRsaKeyCommand();
    bool uploadDir(
            const QString &dirFrom,
            const QString &host,
            const QString &hostDirPath,
            const QString &userName,
            const QString &pwd,
            const QString &rsaFilePath,
            bool ignoreExisting = false,
            bool useSshPass = true); // Will only copy if file is new OR different
    QString getCommand(
            const QString &dirFrom,
            const QString &host,
            const QString &hostDirPath,
            const QString &userName,
            const QString &pwd,
            const QString &rsaFilePath,
            bool ignoreExisting = false,
            bool useSshPass = true) const;
};

#endif // SCPMANAGER_H
