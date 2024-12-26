#ifndef TranslabteTableModel_H
#define TranslabteTableModel_H


#include <QAbstractTableModel>
#include <QDir>

class TranslateTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TranslateTableModel(QObject *parent = nullptr);
    explicit TranslateTableModel(
            const QStringList &texts,
            const QStringList &langs,
            QObject *parent = nullptr);
    explicit TranslateTableModel(
            const QSet<QStringView> &texts,
            const QStringList &langs,
            QObject *parent = nullptr);

    const QStringList &header() const;
    QList<QStringList> getWeirdTranslations() const;
    QList<QStringList> pickTranslationsToCheck(
            int number = 3,
            const QList<QStringList> &listOfStringList = QList<QStringList>{}) const;
    QString createChatGptPromptCheck(const QList<QStringList> &listOfStringList) const;


    bool areAllTranslationsDone() const;
    QStringList getLangCodes() const;
    void pasteTranslatedText(
            const QModelIndex &index,
            const QStringList &lines,
            bool force = false);
    QStringList getWordsToTranslate() const;
    QStringList getTranslations(const QString &lang) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void saveTemporary() const;
    void saveBackup() const;
    void clearTemporary();
    void loadTemporary();
    void loadBackupAfterCrash();

private:
    static const int N_PREVIOUS_TRANS_SIGNATURE;
    static const int PERCENTAGE_TRANS_SAME;
    static const QString KEY_PREVIOUS_TRANS;
    static const QString KEY_TEMPORARY_HEADER;
    static const QString KEY_TEMPORARY_HEADER_BACKUP;
    static const QString KEY_TEMPORARY_LANGS;
    static const QString KEY_TEMPORARY_LANGS_BACKUP;
    static const QString KEY_TEMPORARY_TRANSLATION;
    static const QString KEY_TEMPORARY_TRANSLATION_BACKUP;
    QList<QStringList> m_listOfStringList;
    void _initHeader(const QStringList &langs);
    void _loadTemporary(const QString &settingKey, const QString &settingKeyHeader, const QString &settingKeyLang);
    void _clear();
    QStringList m_header;
    QStringList m_langs;
    QString _settingsKeyPreviousTrans(const QString &langTo) const;
    QList<QStringList> _getPreviousTransSignature(const QString &langTo) const;
    void _saveCurrentSignature(
            const QString &langTo, const QStringList &keywordsTranslated) const;
    QList<QStringList> _countNumberSameAsPrevious(
            const QString &langTo, const QStringList &keywordsTranslated) const;
    int _countNumberSameAsAlreadyPasted(const QStringList &keywordsTranslated) const;
};

#endif // TrTextManager_H
