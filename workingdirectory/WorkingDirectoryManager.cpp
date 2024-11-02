#include <QSettings>

#include "WorkingDirectoryManager.h"

//----------------------------------------
int WorkingDirectoryManager::N_RECENT_MAX = 10;
QString WorkingDirectoryManager::KEY_RECENT = "recents-";
//----------------------------------------
WorkingDirectoryManager::WorkingDirectoryManager()
{
}
//----------------------------------------
WorkingDirectoryManager::~WorkingDirectoryManager()
{
}
//----------------------------------------
WorkingDirectoryManager *WorkingDirectoryManager::instance()
{
    static WorkingDirectoryManager instance;
    return &instance;
}
//----------------------------------------
QSharedPointer<QSettings> WorkingDirectoryManager::settings() const
{
    return QSharedPointer<QSettings>{new QSettings{settingFilePath(),
                                                   QSettings::IniFormat}};
}
//----------------------------------------
QStringList WorkingDirectoryManager::recentlyOpen()
{
    QStringList dirs;
    QSettings settings;
    if (settings.contains(KEY_RECENT)) {
        dirs = settings.value(KEY_RECENT).toStringList();
    }
    return dirs;
}
//----------------------------------------
QString WorkingDirectoryManager::settingFilePath() const
{
    return m_workingDir.filePath("settings.ini");
}
//----------------------------------------
void WorkingDirectoryManager::open(const QString &dir)
{
    setWorkingDir(dir);
}
//----------------------------------------
void WorkingDirectoryManager::open(const QDir &dir)
{
    setWorkingDir(dir);
}
//----------------------------------------
void WorkingDirectoryManager::setWorkingDir(const QString &dir)
{
    open(QDir(dir));
}
//----------------------------------------
void WorkingDirectoryManager::setWorkingDir(const QDir &dir)
{
    m_workingDir = dir;
    QStringList dirs;
    QSettings settings;
    if (settings.contains(KEY_RECENT)) {
        dirs = settings.value(KEY_RECENT).toStringList();
    }
    auto path = dir.path();
    if (!dirs.contains(path)) {
        dirs.insert(0, dir.path());
        settings.setValue(KEY_RECENT, dirs);
    }
}
//----------------------------------------
void WorkingDirectoryManager::removeRecent(const QString &value)
{
    QStringList dirs;
    QSettings settings;
    if (settings.contains(KEY_RECENT)) {
        dirs = settings.value(KEY_RECENT).toStringList();
        dirs.removeOne(value);
        settings.setValue(KEY_RECENT, QVariant::fromValue(dirs));
    }
}
//----------------------------------------
QDir WorkingDirectoryManager::workingDir() const
{
    return m_workingDir;
}
//----------------------------------------
QDir WorkingDirectoryManager::workingDir(const QString &subDirName) const
{
    QDir dir = m_workingDir.filePath(subDirName);
    dir.mkpath(".");
    return dir;
}
//----------------------------------------
QString WorkingDirectoryManager::saveFile(
        const QString &folderName,
        const QString &filePath,
        const QString &newBaseName)
{
    QDir dir = m_workingDir.filePath(folderName);
    dir.cd(folderName);
    dir.mkpath(".");
    QFileInfo fileInfo(filePath);
    QString newFileName = newBaseName + "." + fileInfo.suffix();
    QString newFilePath = dir.filePath(newFileName);
    if (QFile::exists(newFilePath)) {
        QFile::remove(newFilePath);
    }
    QFile::copy(filePath, newFilePath);
    return newFileName;
}
//----------------------------------------
void WorkingDirectoryManager::removeFile(
        const QString &folderName, const QString &fileName)
{
    QDir dir = m_workingDir.filePath(folderName);
    dir.cd(folderName);
    QString filePath = dir.filePath(fileName);
    QFile::remove(filePath);
}
//----------------------------------------
