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
    void addTranslations(const QStringList &textsFrom, // Add translations, creating column if needed
                         const QStringList &textsTranslated,
                         const QString &langFrom,
                         const QString &langTo, bool addingFirstLang);
    int countRows(const QString &langFrom) const;
    int nextPosition(const QString &langFrom) const;
    void translateUsingGoogleApi(const QStringList &texts, const QString &langFrom, const QString &langTo);
    void translateUsingGoogleApi(const QStringList &texts, const QString &langFrom, const QStringList &langsTo);


private:
    QDir m_workingDirectory;
    static const QString COL_COUNT;

    QSqlDatabase getDatabaseOpened(const QString &langFrom) const;
    QString nameDataBase(const QString &langFrom) const;
    QString filePathDataBase(const QString &langFrom) const;
    QString m_lastLangOfDb; // The last lang of the database opened so if a same database is read several time, it will be opened once only
};

#endif // TrTextManager_H
