//*
#include <QNetworkReply>
#include <QSslSocket>
//*/
#include <QNetworkProxy>
#include <QProcess>
#include <QLocale>
    //#include <QLocaleWrapper.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "WebshareProxyManager.h"

//---------------------------------------------
const int WebshareProxyManager::IND_COL_PROXY_HOST = 2;
const int WebshareProxyManager::IND_COL_PROXY_PORT = 3;
const int WebshareProxyManager::IND_COL_PROXY_USERNAME = 4;
const int WebshareProxyManager::IND_COL_PROXY_PWD = 5;
//---------------------------------------------
const QStringList WebshareProxyManager::COL_NAMES
= {QObject::tr("Code"),
   QObject::tr("Country"),
   QObject::tr("Host / IP"),
   QObject::tr("Port"),
   QObject::tr("username"),
   QObject::tr("Password")};
//---------------------------------------------
WebshareProxyManager::WebshareProxyManager(QObject *parent)
    : QAbstractTableModel(parent)
{
    qputenv("QT_SSL_BACKEND", "openssl");
    m_initialized = false;
    m_manager = new QNetworkAccessManager(this);
    connect(&m_timer,
            &QTimer::timeout,
            this,
            &WebshareProxyManager::retrieveProxies);
}
//---------------------------------------------
const QString &WebshareProxyManager::apiKey() const
{
    return m_apiKey;
}
//---------------------------------------------
void WebshareProxyManager::setApiKey(const QString &newApiKey)
{
    m_apiKey = newApiKey;
}
//---------------------------------------------
void WebshareProxyManager::init(const QString &apiKey)
{
    m_apiKey = apiKey;
    retrieveProxies();
}
//---------------------------------------------
void WebshareProxyManager::setAutoRefreshTime(int sec)
{
    if (m_timer.isActive()) {
        m_timer.setInterval(sec * 1000);
    } else {
        m_timer.start(sec * 1000);
    }
}
//---------------------------------------------
void WebshareProxyManager::retrieveProxies()
{
    if (!m_apiKey.isEmpty()) {
        //*
        static QDateTime lastTimeRetrieved = QDateTime::currentDateTime().addSecs(-60*60);
        if (lastTimeRetrieved.secsTo(QDateTime::currentDateTime()) > 260)
        {
            QNetworkProxy noProxy;
            noProxy.setType(QNetworkProxy::NoProxy);

            // Set the application-level proxy to noProxy to cancel using any proxy.
            QNetworkProxy::setApplicationProxy(noProxy);
            QDateTime *p_lastTimeRetrieved = &lastTimeRetrieved;
            QNetworkRequest request(QUrl("https://proxy.webshare.io/api/v2/proxy/list/?mode=direct"));
            QSslConfiguration config = QSslConfiguration::defaultConfiguration();
            config.setProtocol(QSsl::TlsV1_2); // or whichever protocol version you need
            request.setSslConfiguration(config);

            request.setRawHeader("Authorization", ("Token " + m_apiKey).toUtf8());
            QNetworkReply *reply = m_manager->get(request);
            QObject::connect(
                        reply,
                        &QNetworkReply::finished,
                        this,
                        [this, reply, p_lastTimeRetrieved](){
                if (reply->error() == QNetworkReply::NoError) {
                    qDebug() << "SUCCESS retrieving webshare proxy";
                    /*
                QString proxiesString = reply->readAll();
                qDebug() << "REPLY";
                qDebug() << proxiesString;
                proxiesString += " ";
                //*/
                    auto jsonReply = reply->readAll();
                    const QJsonDocument &jsonResponse = QJsonDocument::fromJson(
                                jsonReply);
                    if (jsonResponse.isNull()) {
                        qDebug() << "ERROR webshare init reading JSON";
                        qDebug() << jsonReply;
                        retrieveProxies();
                    } else {
                        const QJsonObject &rootObj = jsonResponse.object();
                        QHash<QString, QNetworkProxy> newIpToProxy;
                        QMultiHash<QString, QString> newCountryIps;
                        QList<QStringList> newListOfStringList;

                        //int count = rootObj.value("count").toInt();
                        const QJsonValue &nextVal = rootObj.value("next");
                        const QString &nextStr = nextVal.isNull() ? "" : nextVal.toString();
                        const QJsonValue &prevVal = rootObj.value("previous");
                        const QString &prevStr = prevVal.isNull() ? "" : prevVal.toString();
                        const QJsonArray &resultsArr = rootObj.value("results").toArray();
                        if (resultsArr.size() > 0) {
                            //beginInsertRows(QModelIndex(), 0, resultsArr.size()-1);
                            for (auto result : resultsArr) {
                                const QJsonObject &resultObj = result.toObject();
                                //QString id = resultObj.value("id").toString();
                                const QString &username = resultObj.value("username").toString();
                                const QString &password = resultObj.value("password").toString();
                                const QString &host = resultObj.value("proxy_address").toString();
                                int port = resultObj.value("port").toInt();
                                //bool valid = resultObj.value("valid").toBool();
                                //QString last_verification = resultObj.value("last_verification").toString();
                                const QString &countryCode = resultObj.value("country_code").toString().toUpper();
                                //QString city_name = resultObj.value("city_name").toString();
                                //QString asn_name = resultObj.value("asn_name").toString();
                                //int asn_number = resultObj.value("asn_number").toInt();
                                //bool high_country_confidence = resultObj.value("high_country_confidence").toBool();
                                //QString created_at = resultObj.value("created_at").toString();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                                QLocale locale(countryCode.toUpper());
                                auto country = locale.country();
#else
                                const auto &country = QLocale::codeToCountry(countryCode);
#endif
                                const QString &countryName = QLocale::countryToString(country);
                                QNetworkProxy proxy(QNetworkProxy::HttpProxy,
                                                    host,
                                                    port,
                                                    username,
                                                    password);
                                newIpToProxy[host] = proxy;
                                newCountryIps.insert(countryCode, host);
                                newListOfStringList << QStringList {
                                                       countryCode,
                                                       countryName,
                                                       host,
                                                       QString::number(port),
                                                       username,
                                                       password};
                            }
                            if (newListOfStringList != m_listOfStringList)
                            {
                                _clear();
                                beginInsertRows(QModelIndex(), 0, resultsArr.size()-1);
                                m_listOfStringList = std::move(newListOfStringList);
                                m_countryIps = std::move(newCountryIps);
                                m_ipToProxy = std::move(newIpToProxy);
                                endInsertRows();
                                *p_lastTimeRetrieved = QDateTime::currentDateTime();
                                emit proxyChanged();
                            }
                        }
                        //endInsertRows();
                        m_initialized = true;
                    }
                } else {
                    qDebug() << "Error retrieving webshare proxy:" << reply->errorString();
                    //QTimer::singleShot(30000, this, &WebshareProxyManager::retrieveProxies);
                }
            });
        }
    //*/






        /*
        QProcess *process = new QProcess;
#ifdef Q_OS_LINUX
        QString program("curl");
#else
        QString program("curl-7.85.0_9-win64-mingw/bin/curl.exe");
#endif
        QStringList arguments;
        arguments << "https://proxy.webshare.io/api/v2/proxy/list/?mode=direct";
        arguments << "--header";
        QString keyArg("Authorization: Token ");
        keyArg += m_apiKey;
        arguments << keyArg;
        connect(process,
        #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        #else
                &QProcess::finished,
        #endif
                this,
                [process, this](int exitCode, QProcess::ExitStatus exitStatus){
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                auto jsonReply = process->readAllStandardOutput();
                QJsonDocument jsonResponse = QJsonDocument::fromJson(
                            jsonReply);
                if (jsonResponse.isNull()) {
                    qDebug() << "ERROR webshare init reading JSON";
                    qDebug() << jsonReply;
                    retrieveProxies();
                } else {
                    QJsonObject rootObj = jsonResponse.object();
                    _clear();

                    //int count = rootObj.value("count").toInt();
                    QJsonValue nextVal = rootObj.value("next");
                    QString nextStr = nextVal.isNull() ? "" : nextVal.toString();
                    QJsonValue prevVal = rootObj.value("previous");
                    QString prevStr = prevVal.isNull() ? "" : prevVal.toString();
                    QJsonArray resultsArr = rootObj.value("results").toArray();
                    if (resultsArr.size() > 0) {
                        beginInsertRows(QModelIndex(), 0, resultsArr.size()-1);
                        for (auto result : resultsArr) {
                            QJsonObject resultObj = result.toObject();
                            //QString id = resultObj.value("id").toString();
                            QString username = resultObj.value("username").toString();
                            QString password = resultObj.value("password").toString();
                            QString host = resultObj.value("proxy_address").toString();
                            int port = resultObj.value("port").toInt();
                            //bool valid = resultObj.value("valid").toBool();
                            //QString last_verification = resultObj.value("last_verification").toString();
                            QString countryCode = resultObj.value("country_code").toString().toUpper();
                            //QString city_name = resultObj.value("city_name").toString();
                            //QString asn_name = resultObj.value("asn_name").toString();
                            //int asn_number = resultObj.value("asn_number").toInt();
                            //bool high_country_confidence = resultObj.value("high_country_confidence").toBool();
                            //QString created_at = resultObj.value("created_at").toString();
                            #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                            QLocale locale(countryCode.toUpper());
                            auto country = locale.country();
                            #else
                            auto country = QLocale::codeToCountry(countryCode);
                            #endif
                            QString countryName = QLocale::countryToString(country);
                            QNetworkProxy proxy(QNetworkProxy::HttpProxy,
                                                host,
                                                port,
                                                username,
                                                password);
                            m_ipToProxy[host] = proxy;
                            m_countryIps.insert(countryCode, host);
                            QStringList line = {countryCode,
                                               countryName,
                                               host,
                                               QString::number(port),
                                               username,
                                               password};
                            m_listOfStringList << line;
                        }
                    }
                    endInsertRows();
                    m_initialized = true;
                    emit proxyChanged();
                }
            } else {
                // Failure
                QByteArray errorOutput = process->readAllStandardError();
                qDebug() << "ERROR webshare init" << exitStatus << exitCode;
                qDebug() << errorOutput;
                retrieveProxies();
            }
        });
        process->start(program, arguments);
        //*/
    }
}
//---------------------------------------------
void WebshareProxyManager::_clear()
{
    bool removedRows = false;
    if (m_listOfStringList.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_listOfStringList.size()-1);
        removedRows = true;
    }
    m_listOfStringList.clear();
    m_domainIps.clear();
    m_domainCountryIps.clear();
    m_proxiesBlocked.clear();
    m_ipToProxy.clear();
    m_countryIps.clear();
    if (removedRows) {
        endRemoveRows();
    }
}
//---------------------------------------------
WebshareProxyManager *WebshareProxyManager::instance()
{
    static WebshareProxyManager instance;
    return &instance;
}
//---------------------------------------------
bool WebshareProxyManager::isInitializedSuccessfully() const
{
    return m_initialized;
}
//---------------------------------------------
QMultiHash<QString, QNetworkProxy> WebshareProxyManager::getProxiesByLang() const
{
    QMultiHash<QString, QNetworkProxy> proxies;
    for (auto it = m_countryIps.begin();
         it != m_countryIps.end(); ++it)
    {
        proxies.insert(it.key(), m_ipToProxy[it.value()]);
    }
    return proxies;
}
//---------------------------------------------
QNetworkProxy WebshareProxyManager::getProxy(
        const QString &domain)
{
    if (!m_domainIps.contains(domain)) {
        m_domainIps[domain] = m_ipToProxy.keys();
    }
    QString ip = m_domainIps[domain].takeFirst();
    m_domainIps[domain] << ip;
    return m_ipToProxy[ip];
}
//---------------------------------------------
QNetworkProxy WebshareProxyManager::getProxy(
        const QString &domain,
        const QString &countryCode)
{
    if (m_countryIps.contains(countryCode)) {
        if (!m_domainCountryIps.contains(domain)
                || m_domainCountryIps[domain].contains(countryCode)) {
            m_domainCountryIps[domain][countryCode]
                    = m_countryIps.values(countryCode);
        }
        QString ip = m_domainCountryIps[domain][countryCode].takeFirst();
        m_domainCountryIps[domain][countryCode] << ip;
        return m_ipToProxy[ip];
    }
    return QNetworkProxy();
}
//---------------------------------------------
bool WebshareProxyManager::hasProxyCountry(
        const QString &countryCode) const
{
    return m_countryIps.contains(countryCode);
}
//---------------------------------------------
void WebshareProxyManager::recordProxyBlock(
        const QString &domain,
        const QString &proxyHost)
{
    m_proxiesBlocked[domain] << proxyHost;
}
//---------------------------------------------
QVariant WebshareProxyManager::headerData(
        int section,
        Qt::Orientation orientation,
        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return COL_NAMES[section];
    }
    return QVariant();
}
//---------------------------------------------
int WebshareProxyManager::rowCount(
        const QModelIndex &) const
{
    return m_listOfStringList.size();
}
//---------------------------------------------
int WebshareProxyManager::columnCount(
        const QModelIndex &) const
{
    return COL_NAMES.size();
}
//---------------------------------------------
QVariant WebshareProxyManager::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_listOfStringList[index.row()][index.column()];
    }
    return QVariant();
}

int WebshareProxyManager::getAutoRefreshTimeSec() const
{
    return m_timer.interval() / 1000;
}
//---------------------------------------------
