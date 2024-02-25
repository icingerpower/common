#ifndef TRANSLATORMANUAL_H
#define TRANSLATORMANUAL_H

#include <QAbstractTableModel>
#include <QDir>

#define KEY_TRANSLATOR_MANUAL_DIR "TranslatorManual_dir"
class TranslatorManual : public QAbstractTableModel
{
    Q_OBJECT

public:
    static QString TODO;
    static TranslatorManual *instance();
    QString getTranslation(const QString &text, const QString &from, const QString &to);
    QDir workingDir() const;
    void setWorkingDir(const QDir &dir);
    void saveEditedTranslation();
    void setShowAll(bool show);
    void save();
    void paste(const QModelIndexList indexes, const QString &text);
    void exportTranslations(const QDir &dir) const;
    void importTranslations(const QDir &dir);

    /*
    bool canDropMimeData(
            const QMimeData *data,
            Qt::DropAction action,
            int row,
            int column,
            const QModelIndex &parent) const override;
    bool dropMimeData(
            const QMimeData *data,
            Qt::DropAction action,
            int row,
            int column,
            const QModelIndex &parent) override;
            //*/
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    explicit TranslatorManual(QObject *parent = nullptr);
    QHash<QString, QHash<QString, QHash<QString, QString>>> m_translations;
    QList<QStringList> m_valuesToTranslate;
    void _saveTranslations(bool onlyTodo = false) const;
    void _loadTranslations();
    void _generateHtml(const QString &filePath, const QStringList &keywords, const QString &langSource) const;
    void _addFromToIfNeeded(const QString &from, QString &to);
    QString _fileName(const QString &from, const QString &to) const;
    QString _fileNameTODO() const;
    bool m_showAll;
};

#endif // TRANSLATORMANUAL_H
