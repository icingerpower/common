#include <QtCore/qmap.h>

#include "FileUtils.h"

//----------------------------------------------------------
FileUtils::FileUtils()
{

}
//----------------------------------------------------------
QStringList FileUtils::fileContentExisting(
        const QStringList &absFileNamesFrom,
        const QStringList &absFileNamesTo)
{
    QStringList absFileNamesExisting;
    for (auto absFileNameFrom : absFileNamesFrom) {
        QFile file(absFileNameFrom);
        file.open(QFile::ReadOnly);
        auto content = file.readAll();
        file.close();
        for (auto absFileNameTo : absFileNamesTo) {
            if (absFileNameFrom == absFileNameTo) {
                absFileNamesExisting << absFileNameFrom;
                continue;
            }
            QFile fileTo(absFileNameTo);
            fileTo.open(QFile::ReadOnly);
            auto importedContent = fileTo.readAll();
            fileTo.close();
            if (content == importedContent && !importedContent.trimmed().isEmpty()) { /// TODO add option to choose to work on empty file or not
                absFileNamesExisting << absFileNameFrom;
            }
        }
    }
    return absFileNamesExisting;
}
//----------------------------------------------------------
QStringList FileUtils::fileContentExisting(
        const QStringList &absFileNamesFrom, const QDir &dir)
{
    QStringList absFileNamesTo;
    auto fileInfos = dir.entryInfoList(QDir::Files);
    for (auto fileInfo : fileInfos) {
        absFileNamesTo << fileInfo.filePath();
    }
    return fileContentExisting(
                absFileNamesFrom,
                absFileNamesTo);
}
//----------------------------------------------------------
bool FileUtils::fileContentExisting(
        const QString &absFileNameFrom, const QDir &dir)
{
    auto existing = fileContentExisting(
                QStringList() << absFileNameFrom, dir);
    return existing.size() > 0;
}
//----------------------------------------------------------
bool FileUtils::fileContentExisting(
        const QString &absFileNameFrom,
        const QStringList &absFileNamesTo)
{
    auto existing = fileContentExisting(
                QStringList() << absFileNameFrom,
                absFileNamesTo);
    return existing.size() > 0;
}
//----------------------------------------------------------
