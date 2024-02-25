#ifndef TEXTTOIMAGE_H
#define TEXTTOIMAGE_H

#include <QString>
#include <QImage>


class TextToImage
{
public:
    static int DEFAULT_WIDTH;
    static int DEFAULT_HEIGHT;
    static QSize DEFAULT_SIZE;
    TextToImage();
    void retrieveImage(
            const QString &text,
            std::function<void(const QList<QImage> &images)> callbackSuccess,
            std::function<void(const QString &error)> callbackError,
            const QString &apiKey = "api-key:quickstart-QUdJIGlzIGNvbWluZy4uLi4K",
            const QString &proxyHost = QString(),
            const QString &proxyPort = QString(),
            const QString &proxyUserName = QString(),
            const QString &proxyPassword = QString(),
            const QString &proxyHostSecond = QString(),
            const QString &proxyPortSecond = QString(),
            const QString &proxyUserNameSecond = QString(),
            const QString &proxyPasswordSecond = QString());
};

#endif // TEXTTOIMAGE_H
