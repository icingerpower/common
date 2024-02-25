#ifndef TRANSLATIONMANUALMANAGER_H
#define TRANSLATIONMANUALMANAGER_H

#include <QAbstractTableModel>
#include <QDir>
#include <QString>
#include <QSet>
#include <QHash>
#include <QStringList>


class TranslationManualManager : public QAbstractTableModel
{
    Q_OBJECT

public:
    static QString KEY_LAST_TRANSLATED;
    static QString KEY_TEMPORARY_TRANSLATION;
    static QString KEY_TEMPORARY_TRANSLATION_BACKUP;
    static TranslationManualManager *instance();
    static QStringList splitText(const QString &text);
    static QSet<int> addSplitText(QStringList &list, const QString &text);
    QDir workingDir() const;
    void init(
            const QDir &workingDir,
            const QString &langCodeFrom,
            const QStringList &langCodesTo,
            const QString &langNameFrom,
            const QStringList &langNamesTo);
    QString getLangCode(int index) const;
    int countHowMuchTranslatedAlready() const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    void pasteTranslatedText(
            const QString &langCode,
            const QStringList &lines);
    void pasteTranslatedTextForcing(
            const QString &langCode,
            const QStringList &lines);
    void saveTemporary() const;
    void loadTemporary();
    void clearTemporary();
    void saveBackup() const;
    void loadBackup();
    void clearBackup();

    void save();
     ///Don't display it yet and won't allow to do the translations
    //bool addTextToTranslate(
            //const QSet<QString> &lines, int maxLines = 1000);
    bool addTextToTranslate(
            const QString &text, int maxLines = 1000);
    int nLinesToTranslate() const;

     ///Display text for translation from what was added before, after removing what is already translated
    void displayTextToTranslate();
    QStringList getDisplayedTextToTranslate() const;
    bool isAllTextAddedTranslated() const;
    //void copyTextTranslated(int colIndex, const QStringList &translations);

     ///Display text translated that contains the string contained. It then possible to edit and change the text
    void displayTextTranslated(
            const QString &contained,
            const QString &langCode,
            Qt::CaseSensitivity cs = Qt::CaseSensitive);
    void clearTextTranslated();

    void loadTranslatedAlready();
    void loadInBufferLangTo(const QString &langTo);
    void trOrAffectBuffered(
            QString &textTo,
            const QString &textFrom,
            bool translate);
    void addTrBufferIfNeeded(
            QString &textToAddTo,
            const QString &textFrom,
            bool translate);
    void getTranslationBufferedSplitting(
            QString &toUpdate,
            const QString &text);
    void addTranslationBufferedSplitting(
            QString &toUpdate,
            const QString &text);
    void translateFromBuffer(QString &toUpdate);
    QString getTranslationBufferedSplitting(
            const QString &text);
    QStringList getTranslationBufferedSplitting(
            const QStringList &lines);
    void getTranslationBuffered(
            QString &toUpdate,
            const QString &text);
    void addTranslationBuffered(
            QString &toUpdate,
            const QString &text);
    QString getTranslationBuffered(
            const QString &text);
    QStringList getTranslationBuffered(
            const QStringList &lines);


protected:
    QString _lastTransSettingFilePath() const;
    void _assertSameNumberOfLines();
    void _clear();
    void _loadTemporary(const QString &settingKey);
    QString _fileName(const QString &langCodeTo) const;
    QDir m_workingDir;
    QString m_langFrom;
    QString m_langToBuffer;
    QStringList m_langsTo;
    QString m_langNameFrom;
    QStringList m_langNamesTo;
    QHash<QString, QString> m_buffer;
    QHash<QString, QStringList> m_lastTranslations;
    QSet<QString> m_linesToTranslate;
    QSet<QString> m_linesTranslatedAlready;
    QList<QStringList> m_tableToTranslate;
    bool m_updateMode;

private:
    explicit TranslationManualManager(QObject *parent = nullptr);
};

#endif // TRANSLATIONMANUALMANAGER_H
