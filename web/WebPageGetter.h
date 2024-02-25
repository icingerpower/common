#ifndef WEBPAGEGETTER_H
#define WEBPAGEGETTER_H

#include <QtCore/qobject.h>

#include <functional>


class WebPageGetter : public QObject
{
    Q_OBJECT
public:
    explicit WebPageGetter(QObject *parent = nullptr);
    QString getPageContent(const QString &url);
    void getGoogleLinks(
            const QString &query,
            const QString &googleDomain,
            std::function<void (const QStringList &)> processLinks,
            bool includeGooglePages = false);
    void getEmails(
            const QStringList &urls,
            std::function<void (const QStringList &)> processEmails);
    void getPageContent(
            const QString &url,
            std::function<void(const QString &)> processContent);
    void getPageText(
            const QString &url,
            std::function<void(const QString &)> processContent);
    //void getRenderedPageText(
            //const QString &url,
            //std::function<void(const QString &)> processContent);
    void waitBeforeNextQuery(const QString &domain, int sec = 91);

signals:

public slots:
};

#endif // WEBPAGEGETTER_H
