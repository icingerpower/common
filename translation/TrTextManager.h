#ifndef TrTextManager_H
#define TrTextManager_H


#include <QAbstractTableModel>
#include <QDir>

//*
class TrTextManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    static QString KEY_LAST_TRANSLATED;
    static QString KEY_TEMPORARY_TRANSLATION;
    static QString KEY_TEMPORARY_TRANSLATION_BACKUP;
    static TrTextManager *instance();
    static TrTextManager *createInstanceForUnitTest();
    //static QStringList splitText(const QString &text);
    static QSet<int> addSplitText(QStringList &lines, const QString &text);
    QDir workingDir() const;
    bool isUpdateMode() const;
    void init(
            const QDir &workingDir,
            const QString &langCodeFrom,
            const QStringList &langCodesTo,
            const QString &langNameFrom,
            const QStringList &langNamesTo);
    QString getLangCode(int index) const;
    virtual int countHowMuchTranslatedAlready() const;
    void loadSomeLines(QStringList &lines, int rowIndex, int colIndex, int nRows);
    void writeSomeLines(QStringList &lines, int rowIndex, int colIndex);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
     ///Display text translated that contains the string contained. It then possible to edit and change the text
    virtual void displayTextTranslated(
            const QString &contained,
            const QString &langCode,
            Qt::CaseSensitivity cs = Qt::CaseInsensitive); // TODO zip
    void clearTextTranslated();

    void pasteTranslatedText(
            const QString &langCode,
            const QStringList &lines,
            bool force = false);
    void pasteTranslatedTextForcing(
            const QString &langCode,
            const QStringList &lines);
    void displayTextToTranslate();
    QStringList getDisplayedTextToTranslate() const;

     ///Display text for translation from what was added before, after removing what is already translated
    bool isAllTextAddedTranslated() const;

    void saveTemporary() const;
    void loadTemporary();
    void clearTemporary();
    void saveBackup() const;
    void loadBackup();
    void clearBackup();

    virtual void convertFromText(); // VIRTUAL
    virtual void save(); // VIRTUAL
    virtual void saveUpdated();  // VIRTUAL

    //bool addTextToTranslate(
            //const QSet<QString> &lines, int maxLines = 1000);
    bool addTextToTranslate(
            const QString &text, int maxLines = 1000);
    int nLinesToTranslate() const;


    virtual void loadTranslatedAlready(); // VIRTUAL
    virtual void loadInBufferLangTo(const QString &langTo); // VIRTUAL
    void analysePotentialDuplicates(
            QMap<int, QStringList> &linesWithDuplicates);
    void trOrAffect(
            QString &textTo,
            const QString &textFrom,
            bool translate);
    void addTrIfNeeded(
            QString &textToAddTo,
            const QString &textFrom,
            bool translate);
    void addTrIfNeeded(
            QList<QVariant> &variants,
            const QString &textFrom,
            bool translate);
    QString getTr(const QString &text) const;
    void getTr(
            QString &toUpdate,
            const QString &text) const;
    void getTr(
            QVariant &toUpdate,
            const QString &text) const;
    void addTr(
            QString &toUpdate,
            const QString &text);
    void addTrIfDone(
            QString &toUpdate,
            const QString &text);
    void addTr(
            QList<QVariant> &variants,
            const QString &text);
    void addTrIfDone(
            QList<QVariant> &variants,
            const QString &text);

protected:
    QDir m_workingDir;
    QString m_langFrom;
    QString m_langToBuffer;
    QStringList m_langsTo;
    QString m_langNameFrom;
    QStringList m_langNamesTo;
    QString _lastTransSettingFilePath() const;
    void _assertSameNumberOfLines();
    void _clear();
    void _loadTemporary(const QString &settingKey);
    QHash<QString, QString> m_buffer;
    QHash<QString, QStringList> m_lastTranslations;
    QSet<QString> m_linesToTranslate;
    QSet<QString> m_linesTranslatedAlready;
    QList<QStringList> m_tableToTranslate;
    bool m_updateMode;

    explicit TrTextManager(QObject *parent = nullptr);
    QList<int> m_indexesColEditedManually;
    QString _fileNameCsv(const QString &langCode) const;
    virtual void readAllLines(QStringList &lines, const QString &langCode) const; // VIRTUAL
    virtual void writeAllLines(const QStringList &lines, const QString &langCode) const; // VIRTUAL
};
//*/

#endif // TrTextManager_H
