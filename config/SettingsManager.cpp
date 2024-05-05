#include <QFile>

#include "SettingsManager.h"

static const QString KEY_PATHS{"settingsOpenedPaths"};

SettingsManager::SettingsManager(QObject *parent)
    : QAbstractListModel(parent)
{
    _loadFromSettings();
}

void SettingsManager::_saveInSettings()
{
    QSettings settings;
    if (m_lastOpenedPaths.size() > 0)
    {
        settings.setValue(KEY_PATHS, m_lastOpenedPaths);
    }
    else if (settings.contains(KEY_PATHS))
    {
        settings.remove(KEY_PATHS);
    }
}

void SettingsManager::_loadFromSettings()
{
    QSettings settings;
    m_lastOpenedPaths = settings.value(KEY_PATHS, QStringList{}).toStringList();
    for (int i=m_lastOpenedPaths.size()-1; i>=0; --i)
    {
        if (!QFile::exists(m_lastOpenedPaths[i]))
        {
            m_lastOpenedPaths.removeAt(i);
        }
    }
}

SettingsManager *SettingsManager::instance()
{
    static SettingsManager settingsManager;
    return &settingsManager;
}

void SettingsManager::open(const QModelIndex &selIndex)
{
    QString filePath = selIndex.data().toString();
    if (QFile::exists(filePath))
    {
        m_openedSettingsPath = filePath;
        int row = selIndex.row();
        if (row > 0)
        {
            m_lastOpenedPaths.removeAt(row);
            m_lastOpenedPaths.insert(0, filePath);
            _saveInSettings();
            emit dataChanged(index(0, 0), index(row, 0));
        }
    }
}

void SettingsManager::open(const QString &filePath)
{
    m_openedSettingsPath = filePath;
    int indexFilePath = m_lastOpenedPaths.indexOf(filePath);
    if (indexFilePath > 0)
    {
        m_lastOpenedPaths.removeAt(indexFilePath);
        m_lastOpenedPaths.insert(0, filePath);
        _saveInSettings();
        emit dataChanged(index(0, 0), index(indexFilePath, 0));
    }
    else if (indexFilePath < 0)
    {
        beginInsertRows(QModelIndex(), 0, 0);
        m_lastOpenedPaths.insert(0, filePath);
        _saveInSettings();
        endInsertRows();
    }
}

QSharedPointer<QSettings> SettingsManager::getSettings() const
{
    QSharedPointer<QSettings> settings{
                new QSettings(m_openedSettingsPath, QSettings::IniFormat)};
    return settings;
}

int SettingsManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_lastOpenedPaths.size();
}

QVariant SettingsManager::data(
        const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole)
    {
        return m_lastOpenedPaths[index.row()];
    }
    return QVariant();
}

bool SettingsManager::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for (int i=row; i<row + count; ++i)
    {
        m_lastOpenedPaths.removeAt(row);
    }
    _saveInSettings();
    endRemoveRows();
    return true;
}
