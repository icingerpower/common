#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QtCore/qstringlist.h>
#include <QtCore/qdir.h>

class FileUtils
{
public:
    FileUtils();
    /// Check if file name or content exists already
    static QStringList fileContentExisting(
            const QStringList &absFileNamesFrom,
            const QStringList &absFileNamesTo);
    static QStringList fileContentExisting(
            const QStringList &absFileNamesFrom,
            const QDir &dir);
    static bool fileContentExisting(
            const QString &absFileNameFrom,
            const QDir &dir);
    static bool fileContentExisting(
            const QString &absFileNameFrom,
            const QStringList &absFileNamesTo);
};

#endif // FILEUTILS_H
