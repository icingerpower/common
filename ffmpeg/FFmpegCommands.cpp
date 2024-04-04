#include <QDir>
#include <QImage>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QCoreApplication>

#include "FFmpegCommands.h"

//----------------------------------------
FFmpegCommands::FFmpegCommands()
{

}
//----------------------------------------
void FFmpegCommands::createSlideshowVideo(
        const QStringList &inFilePaths,
        const QString &tempDir,
        const QString &outVideoFilePath,
        int msec)
{
    QDir tempDirFfmpeg(tempDir);
    tempDirFfmpeg.mkpath(".");
    QList<QImage> images;
    int minWidth = 99999;
    int minHeight = 999999;
    for (auto it=inFilePaths.begin();
         it != inFilePaths.end(); ++it) {
        images << QImage(*it);
        minWidth = qMin(images.last().width(), minWidth);
        minHeight = qMin(images.last().height(), minHeight);
    }
    int i = 1;
    QStringList ffmpegFilePathsToDelete;
    for (auto itIm = images.begin();
         itIm != images.end(); ++itIm) {
        if (itIm->width() > minWidth || minHeight != itIm->height()) {
            *itIm = itIm->scaled(minWidth, minHeight, Qt::IgnoreAspectRatio);
        }
        QString ffmpegFileName("image");
        ffmpegFileName += QString::number(i).rightJustified(2, '0');
        ffmpegFileName += ".jpg";
        QString ffmpegFilePath = tempDirFfmpeg.filePath(ffmpegFileName);
        itIm->save(ffmpegFilePath);
        ffmpegFilePathsToDelete << ffmpegFilePath;
        ++i;
    }

    QStringList args;
    args << "-y"; // Overwrite output file
    args << "-r";
    args << "1/1"; // TODO here for msec
    args << "-i";
    args << "image%2d.jpg"; // Input file pattern
    args << "-r";
    args << "25";
    args << outVideoFilePath;

    // Start ffmpeg process
    QProcess ffmpeg;
    ffmpeg.setWorkingDirectory(tempDir);
    ffmpeg.setProgram("ffmpeg");
    ffmpeg.setArguments(args);
    ffmpeg.start();
    ffmpeg.waitForFinished();
    qDebug() << "RUNNING COMMANDE: ffmpeg " + args.join(" ");
    //qDebug() << ffmpeg.readAllStandardOutput();
    qDebug() << ffmpeg.readAllStandardError();
    for (auto it=ffmpegFilePathsToDelete.begin();
         it != ffmpegFilePathsToDelete.end(); ++it){
        QFile::remove(*it);
    }
}
//----------------------------------------
void FFmpegCommands::extractVideoClip(
        const QString &inVideoFilePath,
        const QString &outVideoFilePath,
        const QPoint &pos,
        const QRect &rect,
        double secBegin,
        double secEnd)
{
    QStringList args;
    args << "-y"; // Overwrite output file
    args << "-i";
    args << inVideoFilePath;
    if (secBegin > 0.001) {
        args << "-ss";
        args << QString::number(secBegin, 'f', 1);
    }
    if (secEnd > 0.001) {
        double duration = secEnd - secBegin;
        args << "-t";
        args << QString::number(duration, 'f', 1);
    }
    if (!pos.isNull() && rect.isValid()) {
        args << "-vf";
        QString cropArg("crop=");
        cropArg += QString::number(rect.width());
        cropArg += ":";
        cropArg += QString::number(rect.height());
        cropArg += ":";
        cropArg += QString::number(pos.x());
        cropArg += ":";
        cropArg += QString::number(pos.y());
        args << cropArg;
    }
    args << outVideoFilePath;

    QProcess ffmpeg;
    ffmpeg.setProgram("ffmpeg");
    ffmpeg.setArguments(args);
    ffmpeg.start();
    ffmpeg.waitForFinished();
    qDebug() << "RUNNING COMMANDE: ffmpeg " + args.join(" ");
    //qDebug() << ffmpeg.readAllStandardOutput();
    qDebug() << ffmpeg.readAllStandardError();
}
//----------------------------------------
void FFmpegCommands::joinVideoClips(
        const QStringList &inFileNames,
        const QString &inFileNameDir,
        const QString &outVideoFilePath)
{
    QSize minSize(99999, 99999);
    QMediaPlayer player;
    for (auto it = inFileNames.begin();
         it != inFileNames.end(); ++it) {
        QString filePath = QDir(inFileNameDir).filePath(*it);
        player.setSource(QUrl::fromLocalFile(filePath));
        player.setPosition(0);
        player.pause();
        qint64 duration = 0.;
        do {
            duration = player.duration();
            QCoreApplication::processEvents();
        } while (duration < 0.001);
        QVariant variantSize
                = player.metaData().value(
                    QMediaMetaData::Resolution);
        QSize size;
        if (variantSize.isValid()) {
            size = variantSize.toSize();
        }
        if (size.width() < minSize.width()) {
            minSize = size;
        } else if (size.width() == minSize.width()
                   && size.height() < minSize.height()) {
            minSize = size;
        }
    }
    Q_ASSERT(minSize.width() < 99999);

    QStringList inFileNamesResized;
    for (auto it = inFileNames.begin();
         it != inFileNames.end(); ++it) {
        QProcess processInter;
        QStringList argInter;
        QString interFileName("inter-");
        interFileName += *it;
        QStringList argsInter;
        argsInter << "-y";
        argsInter << "-i";
        argsInter << *it;
        argsInter << "-vf";

        argsInter << QString("scale=%1:%2")
        //argsInter << QString("scale=%1:-1,crop=%1:%2")
        //argsInter << QString("scale=%1:%2:force_original_aspect_ratio=decrease,crop=%1:%2")
                     .arg(minSize.width())
                     .arg(minSize.height());
        /*
        argsInter << QString("scale=%1:%2").arg(
                        minSize.width())
                    .arg(minSize.height());
        argsInter << "-c:v";
        argsInter << "libx264";
        argsInter << "-preset";
        argsInter << "fast";
        //*/
        argsInter << interFileName;

        QProcess ffmpeg;
        ffmpeg.setProgram("ffmpeg");
        ffmpeg.setWorkingDirectory(inFileNameDir);
        ffmpeg.setArguments(argsInter);
        qDebug() << "RUNNING INTER command: ffmpeg " + argsInter.join(" ");
        ffmpeg.start();
        ffmpeg.waitForFinished();
        qDebug() << ffmpeg.readAllStandardError();

        inFileNamesResized << interFileName;
    }

    QString tempFileName = "temp.txt";
    QString tempFilePath = QDir(inFileNameDir).filePath(tempFileName);
    QFile fileVideoFileNames(tempFilePath);
    if (fileVideoFileNames.open(QFile::WriteOnly)) {
        QTextStream stream(&fileVideoFileNames);
        for (int i=0; i<inFileNamesResized.size(); ++i) {
            if (i == 0) {
                stream << "file '" + inFileNamesResized[i] + "'";
            } else {
                stream << "\nfile '" + inFileNamesResized[i] + "'";
            }

        }
    }
    QStringList args;
    args << "-y"; // Overwrite output file
    args << "-f";
    args << "concat";
    args << "-i";
    args << tempFileName;
    args << "-c";
    args << "copy";

    args << outVideoFilePath;

    QProcess ffmpeg;
    ffmpeg.setProgram("ffmpeg");
    ffmpeg.setWorkingDirectory(inFileNameDir);
    ffmpeg.setArguments(args);
    ffmpeg.start();
    ffmpeg.waitForFinished();
    qDebug() << "RUNNING COMMANDE: ffmpeg " + args.join(" ");
    //qDebug() << ffmpeg.readAllStandardOutput();
    qDebug() << ffmpeg.readAllStandardError();
    QFile::remove(tempFilePath);
    for (auto it = inFileNamesResized.begin();
         it != inFileNamesResized.end(); ++it) {
        QString filePath = QDir(inFileNameDir).filePath(*it);
        QFile::remove(filePath);
    }
}
//----------------------------------------
