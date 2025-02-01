#ifndef WORKINGDIRECTORYMANAGER_H
#define WORKINGDIRECTORYMANAGER_H

#include <QDir>
#include <QLocale>
#include <QSharedPointer>
#include <QSettings>


class WorkingDirectoryManager
{
public:
    static int N_RECENT_MAX;
    static QString KEY_RECENT;
    static const QString KEY_LANGAGE_SEL;
    ~WorkingDirectoryManager();
    static WorkingDirectoryManager *instance();
    QSharedPointer<QSettings> settings() const;
    QSharedPointer<QSettings> settingsLocalIfClient() const;
    QStringList recentlyOpen();
    QString settingsFileName() const;
    QString settingsFilePath() const;
    QString settingsFilePathUnexportable() const;
    void open(const QString &dir);
    void open(const QDir &dir);
    void setWorkingDir(const QString &dir);
    void setWorkingDir(const QDir &dir);
    void removeRecent(const QString &value);
    QDir workingDir() const;
    QDir workingDir(const QString &subDirName) const;
    QString saveFile(const QString &folderName, const QString &filePath, const QString &newBaseName);
    void removeFile(const QString &folderName, const QString &fileName);
    QString workingDirName() const;

    void installDarkPalette();
    void installDarkOrangePalette();
    void installDarkBluePalette();

protected:
    WorkingDirectoryManager();
    QDir m_workingDir;
};

#endif // CONFIGWEBSITES_H
