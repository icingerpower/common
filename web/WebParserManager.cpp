#include <QUrl>
#include <QTimer>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QAuthenticator>

#include "ProxyManager.h"

#include "WebParserManager.h"

//--------------------------------------------------
WebParserManager::WebParserManager(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_msecMinDomain = 1500;
    m_msecMinSameProxy = 21000;
}
//--------------------------------------------------
WebParserManager *WebParserManager::instance()
{
    static WebParserManager instance;
    return &instance;
}
//--------------------------------------------------
void WebParserManager::getContent(
        const QUrl &url,
        std::function<void(const QString &, const QString &content, QNetworkReply *)> callback)
{
    QString domain = QUrl(url).host();
    auto proxy = ProxyManager::instance()->getProxy(domain);
    getContent(url, proxy, callback);
}
//--------------------------------------------------
void WebParserManager::getContent(
        const QUrl &url,
        const QNetworkProxy &proxy,
        std::function<void (const QString &, const QString &, QNetworkReply *)> callback,
        const QHash<QByteArray, QByteArray> &header,
        bool post,
        const QUrlQuery &query)
{
    static QMutex mutex;
    mutex.lock();
    QNetworkAccessManager *networkManager = new QNetworkAccessManager;
    QString domain = url.host();
    bool domainAdded = false;
    if (!m_domainByTimeProxy.contains(domain)) {
        m_domainByTimeProxy[domain] = QHash<QString, QDateTime>();
        m_domainNumberQueries[domain] = 0;
        domainAdded = true;
        m_domainIndexes[domain] = m_domainByTimeProxy.keys().indexOf(domain);
    }
    QString hostName = proxy.hostName();
    QDateTime lastTimeProxy = QDateTime::currentDateTime().addDays(-1);
    QDateTime lastTimeDomain = lastTimeProxy;
    if (m_domainByTimeProxy[domain].contains(hostName)) {
        lastTimeProxy = m_domainByTimeProxy[domain][hostName];
    }
    if (m_domainByTime.contains(domain)) {
        lastTimeDomain = m_domainByTime[domain];
    }
    QDateTime currentDateTime = QDateTime::currentDateTime();
    qint64 msecProxy = lastTimeProxy.msecsTo(currentDateTime);
    qint64 msecDomain = lastTimeProxy.msecsTo(currentDateTime);
    qint64 msecProxyOk = qMax(qint64(0), m_msecMinSameProxy - msecProxy);
    qint64 msecDomainOk = qMax(qint64(0), m_msecMinDomain - msecDomain);
    qint64 msec = qMax(msecProxyOk, msecDomainOk);
    m_domainByTimeProxy[domain][hostName] = QDateTime::currentDateTime().addMSecs(msec);
    m_domainByTime[domain] = QDateTime::currentDateTime().addMSecs(msec);
    mutex.unlock();
    auto lambdaF = [this, networkManager, header, post, query, proxy, domainAdded, domain, hostName, url, callback](){
         //m_domainByTimeProxy[domain][hostName] = QDateTime::currentDateTime();
         //m_domainByTime[domain] = QDateTime::currentDateTime();
         qInfo() << QTime::currentTime().toString("hh:mm:ss") << "Parsing on " << proxy.hostName() << " => " << url;
         ++m_domainNumberQueries[domain];
         int domainIndex = m_domainIndexes[domain];
         if (domainAdded) {
             this->beginInsertRows(QModelIndex(), domainIndex, domainIndex);
             this->endInsertRows();
         } else {
             emit this->dataChanged(
                         this->index(domainIndex, 0, QModelIndex()),
                         this->index(domainIndex, columnCount()-1, QModelIndex()),
                         QVector<int>() << Qt::DisplayRole);
         }
         connect(networkManager,
                 &QNetworkAccessManager::finished,
                 [networkManager, this, callback, url]
                 (QNetworkReply *reply){
             QString content;
             this->_readContent(reply, &content);
             /*
             if (content.contains("isValidAddress\":0")) {
                 auto z1 = reply->request().rawHeaderList();
                 QMap<QString, QString> rawHeadersRequest;
                 for (auto rawHeader : z1) {
                     rawHeadersRequest[rawHeader]
                             = reply->request().rawHeader(rawHeader);
                 }
                 auto url = reply->url();
                 auto z2 = reply->rawHeaderPairs();
                 QMap<QString, QString> rawHeadersReply;
                 for (auto z2it = z2.begin(); z2it != z2.end(); ++z2it) {
                     rawHeadersReply[z2it->first] = z2it->second;
                 }
                 auto error = reply->error();
                 int TEMP = 10;
             }
             //*/
             callback(url.toString(), content, reply);
             networkManager->deleteLater();
         });

         QNetworkRequest request(url);
         QHash<QByteArray, QByteArray> finalHeader;
         if (header.isEmpty()) {
             finalHeader = getRawHeader(url.toString(), proxy);
         } else {
             finalHeader = header;
         }
         for (auto it = finalHeader.begin();
              it != finalHeader.end();
              ++it) {
             request.setRawHeader(it.key(), it.value());
         }
         if (post) {
             auto queryConv = query.toString(QUrl::FullyEncoded).toUtf8();
             networkManager->post(request, query.toString(QUrl::FullyEncoded).toUtf8());
         } else {
             networkManager->get(request);
         }
    };
    if (msec > 0) {
        QTimer::singleShot(msec, lambdaF);
    } else {
        lambdaF();
    }
}
//--------------------------------------------------
QHash<QByteArray, QByteArray> WebParserManager::getRawHeader(
        const QString &url,
        const QNetworkProxy &proxy) const
{
    QHash<QByteArray, QByteArray> header;
    header["User-Ugent"] = getUserAgent(url, proxy);

    //header["Accept-Encoding"] = "gzip, deflate, br";
    //header["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    /*
    header["Accept-Language"] = "en-US,en;q=0.5";
    header["DNT"] = "1";
    header["Connection"] = "keep-alive";
    header["Upgrade-Insecure-Requests"] = "1";
    header["Pragma"] = "no-cache";
    header["Cache-Control"] = "no-cache";
    //*/

    header["Accept"] = "text/html,*/*";
    //*
    header["Accept-Language"] = "en-US,en;q=0.5";
    header["Content-Type"] = "application/x-www-form-urlencoded";
    header["contentType"] = "application/x-www-form-urlencoded;charset=utf-8";
    //header["DNT"] = "1";
    header["Connection"] = "keep-alive";
    QUrl urlqt(url);
    header["Authority"] = urlqt.host().toUtf8();/// domain without https
    header["Origin"] = QString(urlqt.scheme() + "://" + urlqt.host()).toUtf8(); /// domain with https
    header["Referer"] = url.toUtf8(); /// whole url
    header["Path"] = urlqt.path().toUtf8(); /// whole url relative
    header["Cache-Control"] = "no-cache";
    header["Pragma"] = "no-cache";
    header["X-Requested-With"] = "XMLHttpRequest";
    //*/
    return header;
}
//--------------------------------------------------
QByteArray WebParserManager::getUserAgent(
        const QString &, const QNetworkProxy &) const
{
    return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.101 Safari/537.36";
}
//--------------------------------------------------
void WebParserManager::_readContent(QNetworkReply *reply, QString *content) const
{
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
}
//--------------------------------------------------
QVariant WebParserManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QVariantList header = {tr("URL"), tr("Requests"), tr("Last date")};
        return header[section];
    }
    return QVariant();
}
//--------------------------------------------------
int WebParserManager::rowCount(const QModelIndex &) const
{
    return m_domainByTime.size();
}
//--------------------------------------------------
int WebParserManager::columnCount(const QModelIndex &) const
{
    return 3;
}
//--------------------------------------------------
QVariant WebParserManager::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        QString domain = m_domainIndexes.key(index.row());
        if (index.column() == 0) {
            return domain;
        } else if (index.column() == 1) {
            return m_domainNumberQueries[domain];
        } else {
            return m_domainByTime[domain].toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    return QVariant();
}
//--------------------------------------------------
