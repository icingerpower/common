#ifndef TranslabteTableModel_H
#define TranslabteTableModel_H


#include <QAbstractTableModel>
#include <QDir>

class TranslateTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TranslateTableModel(
            const QStringList &texts,
            const QStringList &langs,
            QObject *parent = nullptr);

    void pasteTranslatedText(
            const QModelIndex &index,
            const QStringList &lines,
            bool force = false);
    QStringList getTranslations(const QString &lang) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<QStringList> m_listOfStringList;
    QStringList m_header;
    QStringList m_langs;

};

#endif // TrTextManager_H
