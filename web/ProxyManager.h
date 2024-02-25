#ifndef PROXYMANAGER_H
#define PROXYMANAGER_H

#include <QtCore/qqueue.h>
#include <QtCore/qdatetime.h>
#include <QAbstractListModel>
#include <QNetworkProxy>
#include "../common/utils/SortedMap.h"

class ProxyManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static QString KEY_SETTING_FILE_PATH;
    static ProxyManager *instance();
    QNetworkProxy getProxy(const QString &domain); ///Assume the proxy will be used
    void informProxyBlocked(
            const QString &domain, const QNetworkProxy &proxy); ///Assume the proxy will be used
    void setProxyFilePath(const QString &filePath);
    QString proxyFilePath() const;
    /*
    void setUsername(const QString &userName);
    QString userName() const;
    void setPassword(const QString &password);
    QString password() const;
    void setProxyUls(const QStringList &urls);
    QStringList proxyUrls() const;
    //*/

    // Header:
    QVariant headerData(
            int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int columnCount(
            const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(
            const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(
            const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

private:
    explicit ProxyManager(QObject *parent = nullptr);
    QList<QNetworkProxy> m_allProxies;
    QHash<QString, QQueue<QNetworkProxy>> m_proxiesForDomain;
    //QHash<QString, QHash<QDateTime, QNetworkProxy>> m_proxyDateUsed;
    QHash<QString, QSet<QString>> m_proxiesBlocked;
    SortedMap<QString, QVariant> m_params;
    QString _settingsKey() const;
    QString _settingsKey(int indexParam) const;
    //void _generateDefaultValue();
    void _createProxies();
    //void _loadFromSettings();
    //void _saveInSettings();
};

#endif // PROXYMANAGER_H
