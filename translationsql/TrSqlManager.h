#ifndef TrSqlManager_H
#define TrSqlManager_H


#include <QAbstractTableModel>
#include <QSqlDatabase>
#include <QDir>

class TranslateTableModel;

class TrSqlManager
{
public:
    TrSqlManager(const QString &workingDirectory);
    QString queryTranslation( // Return the translated text from the sqlite database
            const QString &text, const QString &langFrom, const QString &langTo) const;
    QStringList queryTranslations( // Return the translated texts from the sqlite database
            const QStringList &texts, const QString &langFrom, const QString &langTo) const;
    QSet<QString> queryUntranslatedTexts( // Return the texts without translation from the sqlite database
            const QSet<QString> &texts, const QString &langFrom, const QString &langTo) const;
    //QStringList queryUntranslatedTexts( // Return the texts without translation in any languages
            //const QStringList &texts, const QString &langFrom, const QStringList &langsTo) const;
    //QHash<QString, QStringList> queryUntranslatedTextByLang( // Return the texts without translation in all of the languages
            //const QStringList &texts, const QString &langFrom, const QStringList &langsTo) const;
    void addTranslations(const QString &textFrom, // Add translations, creating column if needed
                         const QString &textTranslated,
                         const QString &langFrom,
                         const QString &langTo);
    void addTranslations(const QStringList &textsFrom, // Add translations, creating column if needed
                         const QStringList &textsTranslated,
                         const QString &langFrom,
                         const QString &langTo);
    void translateUsingGoogleApi(const QStringList &texts, const QString &langFrom, const QString &langTo);
    void translateUsingGoogleApi(const QStringList &texts, const QString &langFrom, const QStringList &langsTo);


private:
    QDir m_workingDirectory;

    QSqlDatabase getDatabaseOpened(const QString &langFrom) const;
    QString nameDataBase(const QString &langFrom) const;
    QString filePathDataBase(const QString &langFrom) const;
    QString m_lastLangOfDb; // The last lang of the database opened so if a same database is read several time, it will be opened once only
};

#endif // TrTextManager_H
