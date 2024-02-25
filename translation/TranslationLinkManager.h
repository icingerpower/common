#ifndef TRANSLATIONLINKMANAGER_H
#define TRANSLATIONLINKMANAGER_H

#include <QAbstractTableModel>
#include <QDir>
#include <QString>

class TranslationLinkManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    static TranslationLinkManager *instance();
    static QString FILE_NAME_TRANSLATION;
    QDir workingDir() const;
    void init(
            const QDir &workingDir,
            const QString &langFrom,
            const QStringList &langsTo);
    QString getLink(const QString &link, const QString &langTo) const;
    QHash<QString, QString> getAltLinksByCountryCode(
            const QString &link, const QString &langTo) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void addLinksToTranslate(const QSet<QString> &links);
    void load();
    void save();

private:
    explicit TranslationLinkManager(QObject *parent = nullptr);
    QDir m_workingDir;
    QString m_langFrom;
    QStringList m_langsTo;
    QList<QStringList> m_tableToTranslate;
    QHash<QString, int> m_links;
    QMultiHash<QString, int> m_langCodeToOtherCountryColIndex;
};

#endif // TRANSLATIONLINKMANAGER_H
