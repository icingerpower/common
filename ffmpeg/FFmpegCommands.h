#ifndef FFMPEGCOMMANDS_H
#define FFMPEGCOMMANDS_H

#include <QProcess>

class FFmpegCommands
{
public:
    FFmpegCommands();
    void createSlideshowVideo(
            const QStringList &inFilePaths,
            const QString &tempDir,
            const QString &outVideoFilePath,
            int msec = 1000);
    void extractVideoClip(
            const QString &inVideoFilePath,
            const QString &outVideoFilePath,
            const QPoint &pos,
            const QRect &rect,
            double secBegin,
            double secEnd);
    void joinVideoClips(
            const QStringList &inFileNames,
            const QString &inFileNameDir,
            const QString &outVideoFilePath);

protected:
    QProcess m_process;
};

#endif // FFMPEGCOMMANDS_H
