#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/qeventloop.h>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <QtCore/qtextcodec.h>
#else
#include <QRegularExpression>
#include <QStringDecoder>
#endif
#include <QtCore/qcoreapplication.h>
#include <QtGui/qtextdocument.h>
//#include <QtWebEngineWidgets/qwebenginepage.h>

#include "WebPageGetter.h"
//#include "StrConv.h"

//----------------------------------------------------------
WebPageGetter::WebPageGetter(QObject *parent)
    : QObject(parent)
{
}
//----------------------------------------------------------
QString WebPageGetter::getPageContent(const QString &url)
{
    QNetworkAccessManager networkManager;
    QSharedPointer<QString> content(new QString);
    //*
    QSharedPointer<QEventLoop> loop(new QEventLoop);
    connect(&networkManager,
            &QNetworkAccessManager::finished,
            [content, loop]
            (QNetworkReply *reply){
        //*
        QString encodingHeader = reply->rawHeader(
                    "Content-Type");
        QString encoding = "UTF-8";
        if(encodingHeader.contains("harset="))
        {
            encoding = encodingHeader.split("harset=")[1];
        }

        if(encoding.toLower() == "utf-8")
        {
            *content = QString::fromUtf8(
                        reply->readAll());
        }
        else if (encoding.toUpper() == "ISO-8859-1")
        {
            *content = QString::fromLatin1(
                        reply->readAll());
        }
        else
        {
            Q_ASSERT(false);
        }
        //*/
        loop->quit();
    });
    networkManager.get(
                QNetworkRequest(QUrl(url)));
    loop->exec();
    return *content;
}
//----------------------------------------------------------
void WebPageGetter::waitBeforeNextQuery(
        const QString &domain, int sec)
{
    if (!domain.contains("google")) {
        sec = 61;
    }
    static QHash<QString, QDateTime> lastTimes;
    if (!lastTimes.contains(domain) ) {
        lastTimes[domain] = QDateTime::currentDateTime().addDays(-1);
    }
    QDateTime currentDateTime = QDateTime::currentDateTime();
    qint64 elapsedSec = lastTimes[domain].secsTo(currentDateTime);
    qint64 toSleep = 31;
    if (elapsedSec < sec) {
        toSleep = sec - elapsedSec;
        //SettingManager::sleepSec(sec - elapsedSec);
    //} else {
        //SettingManager::sleepSec(31); // We wait minimum 16 secs for shared server
        QDateTime targetTime
                = QDateTime::currentDateTime().addSecs(sec);
        while (QDateTime::currentDateTime() < targetTime) {
            QCoreApplication::processEvents(
                        QEventLoop::AllEvents, 100);
        }
    }
    lastTimes[domain] = QDateTime::currentDateTime();
    qDebug() << "Waiting done for " << domain << " at " << lastTimes[domain];
}
//----------------------------------------------------------
int googleResultsPageCount_p(const QString &content) {
    int count = 1;
    QStringList elements = content.split("start=");
    elements.takeFirst();
    QList<int> starts;
    for (auto element : elements) {
        if (element.contains("&")) {
            bool ok = false;
            int number = element.split("&")[0].toInt(&ok);
            if (ok) {
                starts << number;
            }
        }
    }
    std::sort(starts.begin(), starts.end());
    int end = 0;
    if (starts.size() > 0) {
        end = starts.last();
    }
    count = end / 100 + 1;
    return count;
}
//----------------------------------------------------------
QStringList googleUrls_p(const QString &content) {
    QStringList urls;
    QStringList elements = content.split("class=\"r\"");
    elements.takeFirst();
    for (auto element : elements)
    {
        QString url = element.split("\"")[1];
        url.replace("/url?q=", "");
        if (!url.contains("cerafmediation.com")
                && !url.contains("photoweb.fr")) {
            urls << url;
        }
    }
    return urls;
}
//----------------------------------------------------------
void WebPageGetter::getGoogleLinks(
        const QString &query,
        const QString &googleDomain,
        std::function<void (const QStringList &)> processLinks,
        bool includeGooglePages)
{
    waitBeforeNextQuery("google." + googleDomain);
    QString encodedQuery = query;
            //= QUrl(query).toString(
                //QUrl::PrettyDecoded).toLatin1().toPercentEncoding();
    while(encodedQuery.contains("  ")) {
        encodedQuery.replace("  ", " ");
    }
    encodedQuery.replace(":", "%3A");
    encodedQuery.replace(" ", "+");
    encodedQuery.replace("@", "%40");
    encodedQuery.replace("(", "%28");
    encodedQuery.replace("|", "%7c");
    encodedQuery.replace(")", "%29");
    QString firstUrl = "https://www.google." + googleDomain + "/search?num=100&q="
            + encodedQuery;
    qDebug() << "WebPageGetter::getGoogleLinks firstUrl:" << firstUrl;
    QStringList *p_urls = new QStringList;
    if (includeGooglePages) {
        *p_urls << firstUrl;
    }
    getPageContent(firstUrl, [=]( //TODO check that copy capture will be ok if initial variable are destroyed before end of exec
                   const QString &content) {
        int nPages = googleResultsPageCount_p(content);
        QStringList firstUrls = googleUrls_p(content);
        *p_urls << firstUrls;
        if (nPages == 1) {
            processLinks(*p_urls);
        } else {
            int *p_nPagesProceed = new int;
            *p_nPagesProceed = 1;
            for (int i=1; i<nPages; ++i) {
                int begin = i*100;
                QString url = "https://www.google." + googleDomain
                        + "/search?num=100&start=" + QString::number(begin)
                        + "&q=" + encodedQuery;
                waitBeforeNextQuery("google." + googleDomain);
                getPageContent(url, [=](
                               const QString &content) {
                    QStringList nextUrls = googleUrls_p(content);
                    qDebug() << "WebPageGetter::getGoogleLinks nextUrl:" << url;
                    static QMutex mutex;
                    mutex.lock();
                    *p_urls << nextUrls;
                    if (includeGooglePages) {
                        *p_urls << url;
                    }
                    ++(*p_nPagesProceed);
                    mutex.unlock();
                    if ((*p_nPagesProceed) == nPages) {
                        processLinks(*p_urls);
                        delete p_nPagesProceed;
                        delete p_urls;
                    }
                });
            }
        }
    });
}
//----------------------------------------------------------
QStringList getEmailsFromContent_p(QString content) {
    Q_ASSERT(!content.contains("302 Moved"));
    //Q_ASSERT(!content.trimmed().isEmpty());
    content = content.simplified();
    content.replace(")", " ");
    content.replace("(", " ");
    content.replace("]", " ");
    content.replace("[", " ");
    content.replace("{", " ");
    content.replace("}", " ");
    content.replace(" at ", "@");
    content.replace("at", "@");
    content.replace(" . ", ".");
    content.replace(" @ ", "@");
    content.replace("@ ", "@");
    content.replace(" @", "@");
    content.replace("<i>", "");
    //content.replace("</i>", "");
    content.replace("<b>", "");
    //content.replace("</b>", "");
    //content.replace("</b", "");
    content.replace("<br>", " ");
    content.replace("<br", " ");
    content.replace(",", " ");
    content.replace(">", " ");
    content.replace("<", " ");
    content.replace("\n", " ");
    content.replace("\\n", " ");
    content.replace("\\n", " ");
    QStringList elements = content.split(" ");
    QStringList emails;
    for (int i=0; i<elements.size(); ++i)
    {
        QString element = elements[i];
        if (element.contains("@"))
        {
            QString previousElement = elements[i>0?i-1:i];
            QString nextElement = elements[i<elements.size()-1?i+1:i];
            QString email = element;
            email = email.split(":").last();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            email.replace(QRegExp("\\.$"), "");
            email.replace(QRegExp("^\\."), "");
            email.replace(QRegExp("^,"), "");
#else
            email.replace(QRegularExpression("\\.$"), "");
            email.replace(QRegularExpression("^\\."), "");
            email.replace(QRegularExpression("^,"), "");
#endif
            email = email.toLower();
            //Q_ASSERT(email.contains("@"));
            //Q_ASSERT(!email.contains(","));
            //Q_ASSERT(!email.contains(";"));
            if (email.count("@") == 1
                    && !email.startsWith("@")
                    && !email.endsWith("@")
                    && !email.contains("/")
                    && !email.contains("%")
                    && !email.contains("\"")
                    && email.contains(".")
                    && !email.contains("..")
                    && !email.contains("!")
                    && !email.contains(",")
                    && !email.contains(";")
                    && !email.contains("*")
                    && !email.contains("?")
                    && !email.contains("@.")
                    && !email.contains("|")
                    && !email.contains("=")
                    && !email.contains("server@")
                    && !email.contains("www.")
                    && (email.endsWith(".fr")
                        || email.endsWith(".com")
                        || email.endsWith(".ca")
                        || email.endsWith(".net")
                        || email.endsWith(".be")
                        || email.endsWith(".ch")))
            {
                emails << email.toLower();
                qDebug() << "Email added:" << email.toLower();
            } else {
                qDebug() << "Wrong email:" << email;
                qDebug() << "Original element:" << element;
            }
        }
    }
    return emails;
}
//----------------------------------------------------------
void WebPageGetter::getEmails(
        const QStringList &urls,
        std::function<void (const QStringList &)> processEmails)
{
    int *nProceed = new int;
    *nProceed = 0;
    for (auto url : urls)
    {
        qDebug() << "WebPageGetter::getEmails of url:" << url;
        QString domain = QUrl(url).host();
        waitBeforeNextQuery(domain);
        if (url.startsWith("http")) {
            getPageText(url, [=](const QString &content) {
                qDebug() << "url:" << url;
                QStringList curEmails = getEmailsFromContent_p(content);;
                processEmails(curEmails);
                ++(*nProceed);
                if (*nProceed == urls.size()) {
                    delete nProceed;
                }
            });
        }
    }
}
//----------------------------------------------------------
void WebPageGetter::getPageContent(
        const QString &url,
        std::function<void (const QString &)> processContent)
{
    QNetworkAccessManager *manager
                = new QNetworkAccessManager;
    connect(manager,
            &QNetworkAccessManager::finished,
            [processContent, manager]
            (QNetworkReply *reply){
        QString encodingHeader = reply->rawHeader("Content-Type");
        QString encoding = "UTF-8";
        if(encodingHeader.contains("harset="))
        {
            encoding = encodingHeader.split("harset=")[1];
        }

        QString content;
        if(encoding.toLower() == "utf-8")
        {
            content = QString::fromUtf8(
                        reply->readAll());
        }
        else if (encoding.toUpper() == "ISO-8859-1")
        {
            content = QString::fromLatin1(
                        reply->readAll());
        }
        else
        {
            Q_ASSERT(false);
        }
        processContent(content);
        manager->deleteLater();
    });
    QNetworkRequest *req = new  QNetworkRequest(QUrl(url));
    req->setMaximumRedirectsAllowed(0);
    manager->get(*req);
    delete req;
}
//----------------------------------------------------------
void WebPageGetter::getPageText(
        const QString &url,
        std::function<void (const QString &)> processContent)
{
     QNetworkAccessManager *manager
                = new QNetworkAccessManager;
    connect(manager,
            &QNetworkAccessManager::finished,
            [processContent, manager]
            (QNetworkReply *reply){
        QString encodingHeader = reply->rawHeader("Content-Type");
        QString encoding = "UTF-8";
        if(encodingHeader.contains("harset="))
        {
            encoding = encodingHeader.split("harset=")[1];
        }

        QString content;
        if(encoding.toLower().contains("utf-8")
                || encoding.toLower().contains("utf8"))
        {
            content = QString::fromUtf8(
                        reply->readAll());
        }
        else if (encoding.toUpper().contains("ISO-8859-1")
                 || encoding.toLower().contains("latin1"))
        {
            content = QString::fromLatin1(
                        reply->readAll());
        }
        else if (encoding.toLower().contains("windows-1252") )
        {
            QByteArray data = reply->readAll();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QTextDecoder* decoder = QTextCodec::codecForName("Windows-1252")->makeDecoder();
            content = decoder->toUnicode(data, data.length());
#else
            QStringDecoder decoder(QStringDecoder::Latin1);
            content = decoder(data);
#endif
        }
        else if (encoding.toLower().contains("binary") )
        {
            content = "";
        }
        else
        {
            Q_ASSERT(false);
        }
        QTextDocument document;
        document.setHtml(content);
        content = document.toPlainText();
        processContent(content);
        manager->deleteLater();
    });
    manager->get(QNetworkRequest(QUrl(url)));

}
/*
//----------------------------------------------------------
void WebPageGetter::getRenderedPageText(
        const QString &url,
        std::function<void (const QString &)> processContent)
{
    QWebEnginePage *webPage = new QWebEnginePage;
    connect(webPage, &QWebEnginePage::loadFinished,
            [webPage, processContent](bool success){
        if(success)
        {
            webPage->toPlainText(
                        [webPage, processContent](
                                 const QString &plainText){
                processContent(plainText);
                webPage->deleteLater();
            });
        }
    });
    webPage->load(url);
}
//*/
//----------------------------------------------------------
