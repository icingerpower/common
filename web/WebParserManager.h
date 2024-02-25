#ifndef WEBPARSERMANAGER_H
#define WEBPARSERMANAGER_H

#include <QAbstractItemModel>
#include <QHash>
#include <QDateTime>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QUrlQuery>

class WebParserManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    static WebParserManager *instance();
    void getContent(const QUrl &url,
            std::function<void(const QString &, const QString&, QNetworkReply *reply)> callback);
    void getContent(const QUrl &url,
            const QNetworkProxy &proxy,
            std::function<void(const QString &, const QString&, QNetworkReply *reply)> callback,
            const QHash<QByteArray, QByteArray> &header = QHash<QByteArray, QByteArray>(),
            bool post = false,
            const QUrlQuery &query = QUrlQuery());
    QHash<QByteArray, QByteArray> getRawHeader(const QString &url, const QNetworkProxy &proxy) const;
    QByteArray getUserAgent(const QString &url, const QNetworkProxy &proxy) const;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;


    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    explicit WebParserManager(QObject *parent = nullptr);
    void _readContent(QNetworkReply *reply, QString *content) const;
    QMap<QString, QHash<QString, QDateTime>> m_domainByTimeProxy;
    QMap<QString, QDateTime> m_domainByTime;
    QMap<QString, int> m_domainNumberQueries;
    QHash<QString, int> m_domainIndexes;
    qint64 m_msecMinDomain;
    qint64 m_msecMinSameProxy;
};

#endif // WEBPARSERMANAGER_H
