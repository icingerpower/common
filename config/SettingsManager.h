#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QAbstractListModel>
#include <QSettings>
#include <QSharedPointer>

class SettingsManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static SettingsManager *instance();
    void open(const QModelIndex &index);
    void open(const QString &filePath);
    QSharedPointer<QSettings> getSettings() const;

    // Basic functionality:
    int rowCount(
            const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(
            const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Remove data:
    bool removeRows(
            int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    explicit SettingsManager(QObject *parent = nullptr);
    QString m_openedSettingsPath;
    QStringList m_lastOpenedPaths;
    void _saveInSettings();
    void _loadFromSettings();
};

#endif // SETTINGSMANAGER_H
