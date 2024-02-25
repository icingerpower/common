#ifndef WEBSHAREPROXYMANAGER_H
#define WEBSHAREPROXYMANAGER_H

#include <QAbstractTableModel>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QSet>
#include <QTimer>

class WebshareProxyManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    static const QStringList COL_NAMES;
    static const int IND_COL_PROXY_HOST;
    static const int IND_COL_PROXY_PORT;
    static const int IND_COL_PROXY_USERNAME;
    static const int IND_COL_PROXY_PWD;
    static WebshareProxyManager *instance();

    bool isInitializedSuccessfully() const;
    QMultiHash<QString, QNetworkProxy> getProxiesByLang() const;
    QNetworkProxy getProxy(
            const QString &domain); ///Assume the proxy will be used
    QNetworkProxy getProxy(
            const QString &domain,
            const QString &countryCode); ///Assume the proxy will be used
    bool hasProxyCountry(const QString &countryCode) const;
    void recordProxyBlock(
            const QString &domain, const QString &proxyHost);

    // Header:
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int getAutoRefreshTimeSec() const;
    const QString &apiKey() const;
    void setApiKey(const QString &newApiKey);

public slots:
    void init(const QString &apiKey);
    void setAutoRefreshTime(int sec);
    void retrieveProxies();

signals:
    void proxyChanged();

private:
    void _clear();
    explicit WebshareProxyManager(QObject *parent = nullptr);
    bool m_initialized;
    QList<QStringList> m_listOfStringList;
    QHash<QString, QStringList> m_domainIps;
    QHash<QString, QHash<QString, QStringList>> m_domainCountryIps;

    QHash<QString, QSet<QString>> m_proxiesBlocked;
    QHash<QString, QNetworkProxy> m_ipToProxy;
    QMultiHash<QString, QString> m_countryIps;
    QNetworkAccessManager *m_manager;
    QString m_apiKey;
    QTimer m_timer;
};

#endif // WEBSHAREPROXYMANAGER_H
