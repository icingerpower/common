#ifndef LANGMANAGER_H
#define LANGMANAGER_H

#include <QAbstractListModel>

class LangManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LangManager(QObject *parent = nullptr);

    static const QHash<QString, QString> COUNTRY_TO_LANG;
    bool hasTranslation(const QString &langCode) const;

    QStringList langCodesTo() const;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool containsLang(const QString &langCode) const;
    //void addLang(const QString &langCode, const QString &langName);
    //void removeLang(const QModelIndex &index);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    //static const QString KEY_SETTINGS_LANGS;
    QString m_settingsFilePath;
    QList<QStringList> m_listOfStringList;
};

#endif // LANGMANAGER_H
