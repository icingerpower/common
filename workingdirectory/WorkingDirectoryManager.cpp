#include <QSettings>
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

#include "WorkingDirectoryManager.h"

//----------------------------------------
int WorkingDirectoryManager::N_RECENT_MAX = 10;
QString WorkingDirectoryManager::KEY_RECENT = "recents-";
const QString WorkingDirectoryManager::KEY_LANGAGE_SEL{"LangageSel"};
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
    return QSharedPointer<QSettings>{new QSettings{settingsFilePath(),
                    QSettings::IniFormat}};
}

QSharedPointer<QSettings> WorkingDirectoryManager::settingsLocalIfClient() const
{
#ifdef CLIENT
    return QSharedPointer<QSettings>{new QSettings{}};
#else
    return settings();
#endif
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
QString WorkingDirectoryManager::settingsFileName() const
{
    return "settings.ini";
}
//----------------------------------------
QString WorkingDirectoryManager::settingsFilePath(
        const QString &customNameIni) const
{
    if (customNameIni.endsWith(".ini", Qt::CaseInsensitive))
    {
        return m_workingDir.filePath(customNameIni);
    }
    return m_workingDir.filePath(customNameIni + ".ini");
}
//----------------------------------------
QString WorkingDirectoryManager::settingsFilePath() const
{
    return m_workingDir.filePath(settingsFileName());
}
//----------------------------------------
QString WorkingDirectoryManager::settingsFilePathUnexportable() const
{
    return m_workingDir.filePath("settingsNoExport.ini");
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
void WorkingDirectoryManager::installDarkPalette()
{
    //QColor darkColor{53, 53, 53};
    //QColor darkerColor{25, 25, 25};
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QColor darkColor{66, 67, 68};
    QColor darkerColor{46, 47, 48};
    QColor modernBlue(104,144,236);
    QPalette palette;
    palette.setColor(QPalette::Window, darkColor);
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, darkerColor);
    palette.setColor(QPalette::AlternateBase,
                     darkColor);
    palette.setColor(QPalette::Light, Qt::white);
    palette.setColor(QPalette::Midlight, Qt::white);
    palette.setColor(QPalette::ToolTipBase, darkerColor);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, darkColor);
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Highlight, modernBlue);
    palette.setColor(QPalette::HighlightedText, Qt::black);
    palette.setColor(QPalette::Disabled,
                     QPalette::Text, Qt::lightGray);
    palette.setColor(QPalette::Disabled,
                     QPalette::ButtonText, Qt::lightGray);
    palette.setColor(QPalette::Disabled,
                     QPalette::WindowText, Qt::lightGray);
    static_cast<QApplication*>(QApplication::instance())->setPalette(
                palette);
}
//----------------------------------------
void WorkingDirectoryManager::installDarkOrangePalette()
{
    installDarkPalette();
    QApplication *app = static_cast<QApplication*>(QApplication::instance());
    QPalette palette = app->palette();
    QColor modernOrange(236,163,104);
    palette.setColor(QPalette::Highlight, modernOrange);
    app->setPalette(palette);
    QString styleSheet = app->styleSheet();
    styleSheet += "QSplitter::handle {background:" + modernOrange.name() + "}";
    styleSheet += "QTableView::indicator::unchecked {"
                  "    border: 1px solid gray;"  // Checkbox border
                  "}"
                  "QListView::indicator::unchecked {"
                  "    border: 1px solid gray;"  // Checkbox border
                  "}"
                  "QTreeView::indicator::unchecked {"
                  "    border: 1px solid gray;"  // Checkbox border
                  "}"
                  "QTableWidget::indicator::unchecked {"
                  "    border: 1px solid gray;"  // Checkbox border
                  "}"
                  "QListWidget::indicator::unchecked {"
                  "    border: 1px solid gray;"  // Checkbox border
                  "}"
                  "QTreeWidget::indicator::unchecked {"
                  "    border: 1px solid gray;"  // Checkbox border
                  "}";
    app->setStyleSheet(styleSheet);
}
//----------------------------------------
void WorkingDirectoryManager::installDarkBluePalette()
{
    installDarkPalette();
    QApplication *app = static_cast<QApplication*>(QApplication::instance());
    QPalette palette = app->palette();
    QColor modernBlue(104,144,236);
    palette.setColor(QPalette::Highlight, modernBlue);
    app->setPalette(palette);
}
//----------------------------------------
