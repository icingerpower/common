#ifndef TrTextManager_H
#define TrTextManager_H


#include <QAbstractTableModel>
#include <QSqlDatabase>
#include <QDir>

class TranslateTableModel;

class TrSqlManager
{
    Q_OBJECT
public:
    TrSqlManager(const QString &workingDirectory);
    QString queryTranslation( // Return the translated text from the sqlite database
            const QString &text, const QString &langFrom, const QString &langTo);
    QStringList queryTranslations( // Return the translated texts from the sqlite database
            const QStringList &texts, const QString &langFrom, const QString &langTo);
    QStringList queryUntranslatedTexts( // Return the texts without translation from the sqlite database
            const QStringList &texts, const QString &langFrom, const QString &langTo);
    QStringList queryUntranslatedTexts( // Return the texts without translation in any languages
            const QStringList &texts, const QString &langFrom, const QStringList &langsTo);
    QHash<QString, QStringList> queryUntranslatedTextByLang( // Return the texts without translation in all of the languages
            const QStringList &texts, const QString &langFrom, const QStringList &langsTo);
    void addTranslations(const QString &textsFrom, // Add translations, creating column if needed
                         const QString &textsTranslated,
                         const QString &langFrom,
                         const QString &langTo);
    TranslateTableModel *createTranslatableModel(
            const QString &texts, const QStringList &langs, QObject *parent = nullptr);
    void translateUsingGoogleApi(const QStringList &texts, const QString &langFrom, const QString &langTo);
    void translateUsingGoogleApi(const QStringList &texts, const QString &langFrom, const QStringList &langsTo);


private:
    QDir m_workingDir;
    QDir m_workingDirectory;

    QSqlDatabase getDatabaseOpened(const QString &langFrom);
    QString nameDataBase(const QString &langFrom);
    QString filePathDataBase(const QString &langFrom);
    QString m_lastLangOfDb; // The last lang of the database opened so if a same database is read several time, it will be opened once only
};

#endif // TrTextManager_H
