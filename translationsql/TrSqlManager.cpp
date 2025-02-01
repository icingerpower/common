#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>

#include "TranslateTableModel.h"
#include "ExceptionTranslation.h"

#include "TrSqlManager.h"

const QString TrSqlManager::COL_COUNT{"position"};

TrSqlManager::TrSqlManager(const QString &workingDirectory)
{
    m_workingDirectory.setPath(workingDirectory);
    if (!m_workingDirectory.exists())
    {
        m_workingDirectory.mkpath(".");
    }
}

QString TrSqlManager::nameDataBase(const QString &langFrom) const
{
    return QString{"translations_%1.db"}.arg(langFrom);
}

QString TrSqlManager::filePathDataBase(const QString &langFrom) const
{
    const auto &fileInfo = m_workingDirectory.entryInfoList(
                QStringList{QString{"translations_%1*db"}.arg(langFrom)},
                QDir::Files, QDir::Name);
    if (fileInfo.size() > 0)
    {
        return fileInfo.last().absoluteFilePath();
    }
    return m_workingDirectory.filePath(nameDataBase(langFrom));
}

QSqlDatabase TrSqlManager::getDatabaseOpened(const QString &langFrom) const
{
    QString connectionName = langFrom;

    if (!QSqlDatabase::contains(connectionName))
    {
        const QString &dbFilePath = filePathDataBase(langFrom);
        qDebug() << "Opening sqlite database:" << dbFilePath;

        // Add a database connection with a unique connection name
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(dbFilePath); // Set the database file path

        if (!db.open())
        {
            ExceptionTranslation exception;
            exception.setError(QObject::tr("Can't open the database:") + " " + dbFilePath);
            exception.raise();
        }
        return db;
    }
    else
    {
        // Return the existing database connection
        return QSqlDatabase::database(connectionName);
    }
}

QString TrSqlManager::queryTranslation(
        const QString &text, const QString &langFrom, const QString &langTo) const
{
    QSqlDatabase db = getDatabaseOpened(langFrom);
    if (langTo == "en")
    {
        if (text == "bolsos de fiesta mujer boda")
        {
            int TEMP=10;++TEMP;
        }
        else if (text.contains("fiesta mujer boda", Qt::CaseInsensitive))
        {
            int TEMP=10;++TEMP;
        }
    }

    if (db.isOpen())
    {
        // Check if the column for the target language exists
        QString queryStr = QString("SELECT %1 FROM translations WHERE keyword = ?").arg(langTo);
        QSqlQuery query(db);
        query.prepare(queryStr);
        query.addBindValue(text);
                // We surround the text with '%' on both sides so that the SQL will find it
        // if it appears anywhere in the column.
        bool execOk = query.exec();

        if (execOk && query.next())
        {
            return query.value(0).toString(); // Return the translation
        }
        else
        {
            qDebug() << "Database name:" << db.databaseName();
            qDebug() << "Database isOpen:" << db.isOpen() << ", lastError:" << db.lastError();
            qDebug() << "Query size:" << query.size();
            qDebug() << "Translation not found or error in query:"
                     << query.lastError().text() << " - "
                     << langFrom << "=>" << langTo
                     << " - execOk:" << execOk << " - " << query.executedQuery()
                     << " - " << text;
            qDebug() << "Database name:" << db.databaseName();
        }
    }
    return QString{};
}

QStringList TrSqlManager::queryTranslations(
        const QStringList &texts, const QString &langFrom, const QString &langTo) const
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

QSet<QString> TrSqlManager::queryUntranslatedTexts(
        const QSet<QString> &texts, const QString &langFrom, const QString &langTo) const
{
    QSet<QString> untranslatedTexts;

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
        exception.setError(QObject::tr("Sql database is not open for language:") + " " + langFrom);
        exception.raise();
        return untranslatedTexts; // Return an empty list if the database can't be opened
    }


    // Check if the 'translations' table exists
    if (!db.tables().contains("translations"))
    {
        // Table doesn't exist, return empty set
        return texts;
    }

    // Check if the 'langTo' column exists in the 'translations' table
    QSqlRecord tableRecord = db.record("translations");
    if (tableRecord.indexOf(langTo) == -1)
    {
        // Column doesn't exist, return empty set
        return texts;
    }


    // Build the query string using the IN clause

    // Add placeholders for the number of texts (e.g., "?, ?, ?")
    const int nMaxTexts = 20;
    QList<QSqlQuery> queries;
    queries << QSqlQuery{db};
    QString queryStr = QString{"SELECT keyword, %1 FROM translations WHERE keyword IN ("}.arg(langTo);
    //for (int i=0; i<texts.size(); i+=nMaxTexts)
    //{
    int i=0;
    int iMax = qMin(nMaxTexts, texts.size())-1;
    int nPlaceholders = iMax - i + 1;
    QStringList placeholders{nPlaceholders, QString{"?"}};
    queryStr += placeholders.join(",") + ")";
    bool queryPrepared = queries.last().prepare(queryStr);
    int nPlaceholders2 = 0;
    QList<int> qadded;
    qadded << nPlaceholders;
    for (const QString &text : texts)
    {
        queries.last().addBindValue(text);
        ++nPlaceholders2;
        if (i == iMax)
        {
            iMax = qMin(i+nMaxTexts, texts.size() - 1) ;
            Q_ASSERT(nPlaceholders2 == nPlaceholders);
            nPlaceholders2 = 0;
            nPlaceholders = iMax - i;
            if (i < iMax)
            {
                queries << QSqlQuery{db};
                queryStr = QString{"SELECT keyword, %1 FROM translations WHERE keyword IN ("}.arg(langTo);
                QStringList placeholders{nPlaceholders, QString{"?"}};
                qadded << nPlaceholders;
                queryStr += placeholders.join(",") + ")";
                queryPrepared = queries.last().prepare(queryStr);
                Q_ASSERT(nPlaceholders > 0);
                Q_ASSERT(queryPrepared);
            }
        }
        ++i;
    }

    int qi = 0;
    QSet<QString> translatedKeywords;
    for (auto &query : queries)
    {
        ++qi;
        // Execute the query
        if (!query.exec())
        {
            qDebug() << "Error executing query:" << query.lastError().text();
            return untranslatedTexts;
        }

        // Create a set to store keywords that have translations (non-null/empty)
        while (query.next())
        {
            const auto &keywordVariant = query.value(0);
            const QString &keyword = keywordVariant.toString();
            const auto &translationVariant = query.value(1);
            const QString &translation = translationVariant.toString();

            // If the translation is not empty or null, add the keyword to the set
            if (!translation.isEmpty())
            {
                translatedKeywords.insert(keyword);
            }
        }
    }

    // Add all keywords that are not translated to the untranslatedTexts list
    for (const QString &text : texts)
    {
        if (!translatedKeywords.contains(text))
        {
            untranslatedTexts << text;
        }
    }

    return untranslatedTexts;
}

void TrSqlManager::addTranslations(
        const QStringList &textsFrom,
        const QStringList &textsTranslated,
        const QString &langFrom,
        const QString &langTo,
        bool addingFirstLang)
{
    // Check that textsFrom and textsTranslated have the same size
    if (textsFrom.size() != textsTranslated.size())
    {
        ExceptionTranslation exception;
        exception.setError(QObject::tr("The number of source texts and translated texts must be the same."));
        exception.raise();
        return;
    }

    // Retrieve the database
    QSqlDatabase db = getDatabaseOpened(langFrom);

    if (!db.isOpen())
    {
        ExceptionTranslation exception;
        exception.setError(QObject::tr("SQL database is not open for language: ") + langFrom);
        exception.raise();
        return;
    }

    // Ensure the 'translations' table exists
    QString createTableQueryStr = "CREATE TABLE IF NOT EXISTS translations ("
                                  "keyword TEXT PRIMARY KEY)";
    QSqlQuery createTableQuery(db);
    if (!createTableQuery.exec(createTableQueryStr))
    {
        qDebug() << "Error creating table:" << createTableQuery.lastError().text();
        ExceptionTranslation exception;
        exception.setError(QObject::tr("SQL error creating table: ") + createTableQuery.lastError().text());
        exception.raise();
        return;
    }

    // Check if the target language column exists; if not, add it
    QSqlRecord tableRecord = db.record("translations");
    if (tableRecord.indexOf(COL_COUNT) == -1)
    {
        QString alterTableQueryStr = QString("ALTER TABLE translations ADD COLUMN %1 INTEGER").arg(COL_COUNT);
        QSqlQuery alterTableQuery(db);
        if (!alterTableQuery.exec(alterTableQueryStr))
        {
            qDebug() << "Error adding column:" << alterTableQuery.lastError().text();
            ExceptionTranslation exception;
            exception.setError(QObject::tr("SQL error adding column: ") + alterTableQuery.lastError().text());
            exception.raise();
            return;
        }
    }
    if (tableRecord.indexOf(langTo) == -1)
    {
        QString alterTableQueryStr = QString("ALTER TABLE translations ADD COLUMN %1 TEXT").arg(langTo);
        QSqlQuery alterTableQuery(db);
        if (!alterTableQuery.exec(alterTableQueryStr))
        {
            qDebug() << "Error adding column:" << alterTableQuery.lastError().text();
            ExceptionTranslation exception;
            exception.setError(QObject::tr("SQL error adding column: ") + alterTableQuery.lastError().text());
            exception.raise();
            return;
        }
    }

    // Begin a single transaction for all batch insertions
    if (!db.transaction())
    {
        qDebug() << "Error starting transaction:" << db.lastError().text();
        ExceptionTranslation exception;
        exception.setError(QObject::tr("SQL error starting transaction: ") + db.lastError().text());
        exception.raise();
        return;
    }

    // Prepare the insert or update query
    QString insertQueryStr;
    if (addingFirstLang)
    {
        insertQueryStr = QString(
                    "INSERT INTO translations (keyword, %1, %2) "
                    "VALUES (:keyword, :%1, :%2) "
                    "ON CONFLICT(keyword) DO UPDATE SET %2 = excluded.%2, %1 = excluded.%1"
                    ).arg(COL_COUNT, langTo);
    }
    else
    {
        insertQueryStr = QString(
                    "INSERT INTO translations (keyword, %1) VALUES (:keyword, :%1) "
                    "ON CONFLICT(keyword) DO UPDATE SET %1 = excluded.%1").arg(langTo);
    }

    QSqlQuery insertQuery(db);
    if (!insertQuery.prepare(insertQueryStr))
    {
        qDebug() << "Error preparing insert query:" << insertQuery.lastError().text();
        ExceptionTranslation exception;
        exception.setError(QObject::tr("SQL error preparing insert query: ") + insertQuery.lastError().text());
        exception.raise();
        db.rollback();
        return;
    }

    // Batch size to prevent exceeding parameter limits
    const int batchSize = 500; // Adjust based on your database's parameter limit
    int currentMax = 0;
    if (addingFirstLang)
    {
        currentMax = nextPosition(langFrom);
    }

    int totalTranslations = textsFrom.size();
    for (int offset = 0; offset < totalTranslations; offset += batchSize)
    {
        int currentBatchSize = qMin(batchSize, totalTranslations - offset);

        for (int i = 0; i < currentBatchSize; ++i)
        {
            int index = offset + i;
            const QString &textFrom = textsFrom.at(index);
            const QString &textTranslated = textsTranslated.at(index);

            insertQuery.bindValue(":keyword", textFrom);
            insertQuery.bindValue(":" + langTo, textTranslated);
            if (addingFirstLang)
            {
                insertQuery.bindValue(":" + COL_COUNT, currentMax);
            }

            if (!insertQuery.exec())
            {
                qDebug() << "Error executing insert query for text:" << textFrom << " Error:" << insertQuery.lastError().text();
                ExceptionTranslation exception;
                exception.setError(QObject::tr("SQL error adding translation for text: ") + textFrom + " " + insertQuery.lastError().text());
                exception.raise();
                db.rollback(); // Rollback the transaction
                return;
            }
        }
    }

    // Commit the transaction after all batches are processed
    if (!db.commit())
    {
        qDebug() << "Error committing transaction:" << db.lastError().text();
        ExceptionTranslation exception;
        exception.setError(QObject::tr("SQL error committing transaction: ") + db.lastError().text());
        exception.raise();
        db.rollback();
        return;
    }
}

int TrSqlManager::countRows(const QString &langFrom) const
{
    QSqlDatabase db = getDatabaseOpened(langFrom);
    if (!db.isOpen())
    {
        qDebug() << "Database is not open!";
        return 0;
    }

    // Prepare the query to count rows
    QSqlQuery query(db);
    QString countQueryStr = "SELECT COUNT(*) FROM translations";

    if (!query.exec(countQueryStr))
    {
        qDebug() << "Error counting rows:" << query.lastError().text();
        return 0;
    }

    // Retrieve the count
    if (query.next())
    {
        int count = query.value(0).toInt();
        return count;
    }
    else
    {
        qDebug() << "Error retrieving row count";
        return 0;
    }
}

int TrSqlManager::nextPosition(const QString &langFrom) const
{
    // Get the database
    QSqlDatabase db = getDatabaseOpened(langFrom);
    if (!db.isOpen())
    {
        qDebug() << "Database is not open!";
        return 1; // Default to 1
    }

    // Prepare the query to find the max position
    QSqlQuery query(db);
    QString maxPositionQueryStr = QString{"SELECT COALESCE(MAX(%1), 0) + 1 FROM translations"}.arg(COL_COUNT);

    if (!query.exec(maxPositionQueryStr))
    {
        qDebug() << "Error getting next position:" << query.lastError().text();
        return 1; // Default to 1
    }

    // Retrieve the next position
    if (query.next())
    {
        return query.value(0).toInt();
    }
    else
    {
        qDebug() << "Error retrieving max position";
        return 1; // Default to 1
    }
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


