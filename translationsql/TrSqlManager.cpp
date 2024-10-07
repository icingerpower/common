#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include "TranslateTableModel.h"
#include "ExceptionTranslation.h"

#include "TrSqlManager.h"

TrSqlManager::TrSqlManager(const QString &workingDirectory)
{
    m_workingDirectory.setPath(workingDirectory);
    if (!m_workingDir.exists())
    {
        m_workingDir.mkpath(".");
    }
}

QString TrSqlManager::nameDataBase(const QString &langFrom)
{
    return QString{"translations_%1.db"}.arg(langFrom);
}

QString TrSqlManager::filePathDataBase(const QString &langFrom)
{
    return m_workingDir.filePath(nameDataBase(langFrom));
}

QSqlDatabase TrSqlManager::getDatabaseOpened(const QString &langFrom)
{
    if (m_lastLangOfDb != langFrom)
    {
        const QString &dbFilePath = filePathDataBase(langFrom);
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dbFilePath);
        if (!db.open())
        {
            ExceptionTranslation exception;
            exception.setError(tr("Can’t open the database:") + " " + dbFilePath);
            exception.raise();
        }
        return db;
    }
    return QSqlDatabase::database();
}

QString TrSqlManager::queryTranslation(
        const QString &text, const QString &langFrom, const QString &langTo)
{
    QSqlDatabase db = getDatabaseOpened(langFrom);

    if (db.isOpen())
    {
        // Check if the column for the target language exists
        QString queryStr = QString("SELECT %1 FROM translations WHERE keyword = ?").arg(langTo);
        QSqlQuery query(db);
        query.prepare(queryStr);
        query.addBindValue(text);

        if (query.exec() && query.next())
        {
            return query.value(0).toString(); // Return the translation
        }
        else
        {
            qDebug() << "Translation not found or error in query:" << query.lastError().text();
        }
    }
    return QString{};
}

QStringList TrSqlManager::queryTranslations(
        const QStringList &texts, const QString &langFrom, const QString &langTo)
{
    QStringList translations;

    // Check if the texts list is empty
    if (texts.isEmpty())
    {
        return translations; // Return an empty list if no texts are provided
    }

    // Retrieve the correct database
    QSqlDatabase db = getDatabaseOpened(langFrom);

    if (!db.isOpen())
    {
        return translations; // Return an empty list if the database can't be opened
    }

    // Build the query string using the IN clause
    QString queryStr = QString("SELECT keyword, %1 FROM translations WHERE keyword IN (").arg(langTo);

    // Add placeholders for the number of texts (e.g., "?, ?, ?")
    QStringList placeholders;
    for (int i = 0; i < texts.size(); ++i) {
        placeholders << "?";
    }
    queryStr += placeholders.join(", ") + ")";

    QSqlQuery query(db);
    query.prepare(queryStr);

    // Bind each keyword to the query
    for (const QString &text : texts)
    {
        query.addBindValue(text);
    }

    // Execute the query
    if (!query.exec())
    {
        qDebug() << "Error executing query:" << query.lastError().text();
        return translations;
    }

    // Create a map to hold translations keyed by the original keyword
    QMap<QString, QString> translationMap;
    while (query.next())
    {
        QString keyword = query.value(0).toString();
        QString translation = query.value(1).toString();
        translationMap[keyword] = translation;
    }

    // Fill the translations list in the order of the original texts
    for (const QString &text : texts)
    {
        translations << translationMap.value(text, QString{}); // Default to empty string if no translation found
    }

    return translations;
}

QStringList TrSqlManager::queryUntranslatedTexts(
        const QStringList &texts, const QString &langFrom, const QString &langTo)
{
    QStringList untranslatedTexts;

    // Check if the texts list is empty
    if (texts.isEmpty())
    {
        return untranslatedTexts; // Return an empty list if no texts are provided
    }

    // Retrieve the correct database
    QSqlDatabase db = getDatabaseOpened(langFrom);

    if (!db.isOpen())
    {
        ExceptionTranslation exception;
        exception.setError(tr("Sql database is not open for language:") + " " + langFrom);
        exception.raise();
        return untranslatedTexts; // Return an empty list if the database can't be opened
    }

    // Build the query string using the IN clause
    QString queryStr = QString("SELECT keyword, %1 FROM translations WHERE keyword IN (").arg(langTo);

    // Add placeholders for the number of texts (e.g., "?, ?, ?")
    QStringList placeholders;
    for (int i = 0; i < texts.size(); ++i)
    {
        placeholders << "?";
    }
    queryStr += placeholders.join(", ") + ")";

    QSqlQuery query(db);
    query.prepare(queryStr);

    // Bind each keyword to the query
    for (const QString &text : texts)
    {
        query.addBindValue(text);
    }

    // Execute the query
    if (!query.exec())
    {
        qDebug() << "Error executing query:" << query.lastError().text();
        return untranslatedTexts;
    }

    // Create a set to store keywords that have translations (non-null/empty)
    QSet<QString> translatedKeywords;
    while (query.next())
    {
        QString keyword = query.value(0).toString();
        QString translation = query.value(1).toString();

        // If the translation is not empty or null, add the keyword to the set
        if (!translation.isEmpty())
        {
            translatedKeywords.insert(keyword);
        }
    }

    // Add all keywords that are not translated to the untranslatedTexts list
    for (const QString &text : texts)
    {
        if (!translatedKeywords.contains(text)) {
            untranslatedTexts << text;
        }
    }

    return untranslatedTexts;
}

QStringList TrSqlManager::queryUntranslatedTexts(
        const QStringList &texts,
        const QString &langFrom,
        const QStringList &langsTo)
{
    QSet<QString> untranslatedSet;
    for (const auto &langTo : langsTo)
    {
        const QStringList &untranslatedTexts
                = queryUntranslatedTexts(
                    texts, langFrom, langTo);
        if (untranslatedSet.isEmpty())
        {
            for (const auto &text : untranslatedTexts)
            {
                untranslatedSet.insert(text);
            }
        }
        else
        {
            QSet<QString> set{untranslatedTexts.begin(), untranslatedTexts.end()};
            untranslatedSet.intersect(set);
        }
    }
    return QStringList{untranslatedSet.begin(), untranslatedSet.end()};
}

QHash<QString, QStringList> TrSqlManager::queryUntranslatedTextByLang(
        const QStringList &texts,
        const QString &langFrom,
        const QStringList &langsTo)
{
    QHash<QString, QStringList> untranslated;
    for (const auto &langTo : langsTo)
    {
        untranslated[langTo]
                = queryUntranslatedTexts(
                    texts, langFrom, langTo);
    }
    return untranslated;
}

void TrSqlManager::addTranslations(
        const QString &textFrom,
        const QString &textTranslated,
        const QString &langFrom,
        const QString &langTo)
{
    QString dbPath = filePathDataBase(langFrom);
    QSqlDatabase db = getDatabaseOpened(langFrom);

    if (!db.isOpen())
    {
        ExceptionTranslation exception;
        exception.setError(tr("Sql database is not open for language:") + " " + langFrom);
        exception.raise();
    }

    // Ensure the table and columns exist
    QString createTableQueryStr = "CREATE TABLE IF NOT EXISTS translations ("
                                  "keyword TEXT PRIMARY KEY)";
    QSqlQuery createTableQuery(db);
    if (!createTableQuery.exec(createTableQueryStr))
    {
        qDebug() << "Error creating table:" << createTableQuery.lastError().text();
        ExceptionTranslation exception;
        exception.setError(tr("Sql error creating table:") + " " + createTableQuery.lastError().text());
        exception.raise();
        return;
    }

    // Add the target language column if it doesn't exist
    QString alterTableQueryStr = QString("ALTER TABLE translations ADD COLUMN %1 TEXT").arg(langTo);
    QSqlQuery alterTableQuery(db);
    if (!alterTableQuery.exec())
    {
        QString errorText = alterTableQuery.lastError().text();
        if (!errorText.contains("duplicate column name"))
        {
            ExceptionTranslation exception;
            exception.setError(tr("Sql error adding column:") + " " + alterTableQuery.lastError().text());
            exception.raise();
            return;
        }
    }

    // Insert or update the translation
    const QString &insertQueryStr
            = "INSERT INTO translations (keyword, " + langTo + ") VALUES (:keyword, :" + langTo + ") "
              "ON CONFLICT(keyword) DO UPDATE SET " + langTo + " = :" + langTo;
    QSqlQuery insertQuery(db);
    insertQuery.prepare(insertQueryStr);
    insertQuery.bindValue(":keyword", textFrom);
    insertQuery.bindValue(":" + langTo, textTranslated);

    if (!insertQuery.exec())
    {
        ExceptionTranslation exception;
        exception.setError(tr("Sql error adding translation:") + " " + insertQuery.lastError().text());
        exception.raise();
    }
}

TranslateTableModel *TrSqlManager::createTranslatableModel(
        const QString &texts, const QStringList &langs, QObject *parent)
{
    return new TranslateTableModel{texts, langs, parent};
}

void TrSqlManager::translateUsingGoogleApi(
        const QStringList &texts, const QString &langFrom, const QString &langTo)
{
    // TODO
}

void TrSqlManager::translateUsingGoogleApi(
        const QStringList &texts, const QString &langFrom, const QStringList &langsTo)
{
    // TODO
}


