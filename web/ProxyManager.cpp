#include <QtCore/qsettings.h>
#include <QFile>
#include <QSettings>

#include "ExceptionProxyPathNotSet.h"

#include "ProxyManager.h"

//----------------------------------------------------------
QString ProxyManager::KEY_SETTING_FILE_PATH
= "ProxyManager::proxyFilePath";
//----------------------------------------------------------
ProxyManager::ProxyManager(QObject *parent)
    : QAbstractListModel(parent)
{
    //_generateDefaultValue();
    //_loadFromSettings();
    QString filePath = proxyFilePath();
    if (!filePath.isEmpty()) {
        _createProxies();
    }
}
//----------------------------------------------------------
ProxyManager *ProxyManager::instance()
{
    static ProxyManager instance;
    return &instance;
}
//----------------------------------------------------------
void ProxyManager::setProxyFilePath(const QString &filePath)
{
    QSettings settings;
    settings.setValue(KEY_SETTING_FILE_PATH,
                      filePath);
    _createProxies();
    //Q_ASSERT(!m_allProxies.isEmpty());
}
//----------------------------------------------------------
QString ProxyManager::proxyFilePath() const
{
    QSettings settings;
    return settings.value(
                KEY_SETTING_FILE_PATH, "").toString();
}
//----------------------------------------------------------
void ProxyManager::_createProxies()
{
    m_allProxies.clear();
    auto filePath = proxyFilePath();
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                QStringList elements = stream.readLine().split(":");
                QNetworkProxy proxy(
                            QNetworkProxy::HttpProxy,
                            elements[0],
                        elements[1].toUShort(),
                        elements[2],
                        elements[3]);
                m_allProxies << proxy;
            }
            file.close();
        }
    }
}
//----------------------------------------------------------
QNetworkProxy ProxyManager::getProxy(const QString &domain)
{
    QSettings settings;
    if (!settings.contains(KEY_SETTING_FILE_PATH)) {
        ExceptionProxyPathNotSet exception;
        exception.raise();
    } else {
        _createProxies();
    }
    if (!m_proxiesForDomain.contains(domain)) {
        m_proxiesForDomain[domain] = QQueue<QNetworkProxy>();
        m_proxiesForDomain[domain] << m_allProxies;
    }
    auto first = m_proxiesForDomain[domain].takeFirst();
    m_proxiesForDomain[domain] << first;
    for (int i=0; i<10; ++i) {
        if(m_proxiesBlocked.contains(domain)
                && m_proxiesBlocked[domain].contains(first.hostName())) {
            continue;
        }
        auto first = m_proxiesForDomain[domain].takeFirst();
        m_proxiesForDomain[domain] << first;
    }
    return first;
}
//----------------------------------------------------------
void ProxyManager::informProxyBlocked(const QString &domain, const QNetworkProxy &proxy)
{
    if (!m_proxiesBlocked.contains(domain)) {
        m_proxiesBlocked[domain] = QSet<QString>();
    }
    m_proxiesBlocked[domain] << proxy.hostName();
}
//----------------------------------------------------------
QVariant ProxyManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            return m_params.keyByIndex(section);
        } else if (orientation == Qt::Horizontal) {
            return tr("Valeur");
        }
    }
    return QVariant();
}
//----------------------------------------------------------
int ProxyManager::columnCount(const QModelIndex &) const
{
    return 1;
}
//----------------------------------------------------------
int ProxyManager::rowCount(const QModelIndex &) const
{
    return m_params.size();
}
//----------------------------------------------------------
QVariant ProxyManager::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_params.valueByIndex(index.row());
    }
    return QVariant();
}
//----------------------------------------------------------
Qt::ItemFlags ProxyManager::flags(const QModelIndex &) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}
//----------------------------------------------------------
bool ProxyManager::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (role == Qt::EditRole) {
        if (value != m_params.valueByIndex(index.row())) {
            m_params.setValue(index.row(), value);
            //_createProxies();
            //_saveInSettings();
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------
QString ProxyManager::_settingsKey() const
{
    return "ProxyManager_params";
}
//----------------------------------------------------------
QString ProxyManager::_settingsKey(int indexParam) const
{
    return _settingsKey() + "-" + QString::number(indexParam);
}
//----------------------------------------------------------
