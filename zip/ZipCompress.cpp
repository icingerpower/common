//*
#include <zlib.h>

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

#include "ZipCompress.h"

//----------------------------------------
ZipCompress::ZipCompress()
{

}
//----------------------------------------
void ZipCompress::compress(
        const QString &filePathFrom,
        const QString &filePathTo)
{
    QFile fileFrom(filePathFrom);
    QFile fileTo(filePathTo);
    if (!fileFrom.open(QIODevice::ReadOnly))
    {
        qDebug() << "Impossible d'ouvrir le fichier source";
        return;
    }

    if (!fileTo.open(QIODevice::WriteOnly))
    {
        qDebug() << "Impossible d'ouvrir le fichier de destination";
        return;
    }

    gzFile gz = gzopen(QFile::encodeName(filePathTo).constData(), "wb");
    if (gz == nullptr)
    {
        qDebug() << "Erreur lors de l'ouverture du fichier de destination avec zlib";
        return;
    }

    char buffer[1024];
    qint64 bytesRead = 0;
    while ((bytesRead = fileFrom.read(buffer, sizeof(buffer))) > 0)
    {
        gzwrite(gz, buffer, bytesRead);
    }

    gzclose(gz);
    fileFrom.close();
    fileTo.close();

}
//----------------------------------------
void ZipCompress::uncompress(
        const QString &filePathFrom,
        const QString &filePathTo)
{
    QFile fileFrom(filePathFrom);
    QFile fileTo(filePathTo);
    if (!fileFrom.open(QIODevice::ReadOnly))
    {
        qDebug() << "Impossible d'ouvrir le fichier source";
        return;
    }

    if (!fileTo.open(QIODevice::WriteOnly))
    {
        qDebug() << "Impossible d'ouvrir le fichier de destination";
        return;
    }

    gzFile gz = gzopen(QFile::encodeName(filePathFrom).constData(), "rb");
    if (gz == nullptr)
    {
        qDebug() << "Erreur lors de l'ouverture du fichier source avec zlib";
        return;
    }

    char buffer[1024];
    int bytesRead = 0;
    while ((bytesRead = gzread(gz, buffer, sizeof(buffer))) > 0)
    {
        fileTo.write(buffer, bytesRead);
    }

    gzclose(gz);
    fileFrom.close();
    fileTo.close();
}
//----------------------------------------
void ZipCompress::compressFolder(
        const QString &folderPathFrom,
        const QString &filePathTo)
{
    QDir folderDir(folderPathFrom);
    if (!folderDir.exists())
    {
        qDebug() << "Dossier source inexistant";
        return;
    }

    QFile fileTo(filePathTo);
    if (!fileTo.open(QIODevice::WriteOnly))
    {
        qDebug() << "Impossible d'ouvrir le fichier de destination";
        return;
    }

    gzFile gz = gzopen(QFile::encodeName(filePathTo).constData(), "wb");
    if (gz == nullptr)
    {
        qDebug() << "Erreur lors de l'ouverture du fichier de destination avec zlib";
        return;
    }

    QFileInfoList fileList = folderDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &fileInfo, fileList)
    {
        QString filePath = fileInfo.filePath();
        QString relativePath = folderDir.relativeFilePath(filePath);
        if (fileInfo.isDir())
        {
            gzprintf(gz, "%s/", qPrintable(relativePath));
        }
        else if (fileInfo.isFile())
        {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly))
            {
                qDebug() << "Impossible d'ouvrir le fichier" << filePath;
                continue;
            }

            gzprintf(gz, "%s", qPrintable(relativePath));

            char buffer[1024];
            qint64 bytesRead = 0;
            while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0)
            {
                gzwrite(gz, buffer, bytesRead);
            }

            file.close();
        }
    }
    gzclose(gz);
    fileTo.close();
}
//----------------------------------------
void ZipCompress::uncompressFolder(
        const QString &filePathFrom,
        const QString &folderPathTo)
{
    QDir folderDir(folderPathTo);
    if (!folderDir.exists())
    {
        qDebug() << "Dossier de destination inexistant";
        return;
    }

    QFile fileFrom(filePathFrom);
    if (!fileFrom.open(QIODevice::ReadOnly))
    {
        qDebug() << "Impossible d'ouvrir le fichier source";
        return;
    }

    gzFile gz = gzopen(QFile::encodeName(filePathFrom).constData(), "rb");
    if (gz == nullptr)
    {
        qDebug() << "Erreur lors de l'ouverture du fichier source avec zlib";
        return;
    }

    char buffer[1024];
    qint64 bytesRead = 0;
    QString currentFileName;
    QFile currentFile;

    while ((bytesRead = gzread(gz, buffer, sizeof(buffer))) > 0)
    {
        if (currentFile.isOpen())
        {
            currentFile.write(buffer, bytesRead);
        }
        else
        {
            currentFileName.append(QString::fromUtf8(buffer, bytesRead));

            if (currentFileName.endsWith('/'))
            {
                currentFileName.chop(1);
                QDir().mkpath(folderDir.filePath(currentFileName));
            }
            else
            {
                currentFile.setFileName(folderDir.filePath(currentFileName));
                if (!currentFile.open(QIODevice::WriteOnly))
                {
                    qDebug() << "Impossible de créer le fichier" << currentFile.fileName();
                    currentFileName.clear();
                }
            }
        }
    }

    gzclose(gz);
    fileFrom.close();
}
//*/
//----------------------------------------
