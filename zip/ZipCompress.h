#ifndef ZIPCOMPRESS_H
#define ZIPCOMPRESS_H

#include <QString>

class ZipCompress
{
public:
    ZipCompress();
    static void compress(const QString &filePathFrom, const QString &filePathTo);
    static void uncompress(const QString &filePathFrom, const QString &filePathTo);
    static void compressFolder(const QString &folderPathFrom, const QString &filePathTo);
    static void uncompressFolder(const QString &filePathFrom, const QString &folderPathTo);
};

#endif // ZIPCOMPRESS_H
