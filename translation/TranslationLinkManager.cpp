#include <QLocale>
#include <QFile>
#include <QTextStream>

#include "TranslationLinkManager.h"

//----------------------------------------
QString TranslationLinkManager::FILE_NAME_TRANSLATION = "trans-link.csv";
//----------------------------------------
TranslationLinkManager::TranslationLinkManager(QObject *parent)
    : QAbstractTableModel(parent)
{
}
//----------------------------------------
TranslationLinkManager *TranslationLinkManager::instance()
{
    static TranslationLinkManager instance;
    return &instance;
}
//----------------------------------------
QDir TranslationLinkManager::workingDir() const
{
    return m_workingDir;
}
//----------------------------------------
void TranslationLinkManager::init(
        const QDir &workingDir,
        const QString &langFrom,
        const QStringList &langsTo)
{
    m_workingDir = workingDir;
    m_langFrom = langFrom;
    m_langsTo = langsTo;
    QList<QLocale> locales;
    locales << QLocale(QLocale::English, QLocale::UnitedKingdom);
    locales << QLocale(QLocale::English, QLocale::Canada);
    locales << QLocale(QLocale::English, QLocale::Australia);
    locales << QLocale(QLocale::English, QLocale::Malta);
    locales << QLocale(QLocale::English, QLocale::Singapore);
    locales << QLocale(QLocale::English, QLocale::Singapore);
    locales << QLocale(QLocale::French, QLocale::Belgium);
    locales << QLocale(QLocale::French, QLocale::Canada);
    locales << QLocale(QLocale::Spanish, QLocale::Mexico);
    locales << QLocale(QLocale::Spanish, QLocale::Colombia);
    locales << QLocale(QLocale::Portuguese, QLocale::Brazil);
    locales << QLocale(QLocale::Chinese, QLocale::Singapore);
    QMultiHash<QString, QString> langWithcountries;
    for (auto locale = locales.begin();
         locale != locales.end(); ++locale) {
        QString langCode = QLocale::languageToCode(locale->language()).toLower();
        QString localeName = locale->name().replace("_", "-");
        langWithcountries.insert(langCode, localeName);
    }
    QStringList langCodeAll = langsTo;
    langCodeAll.insert(0, langFrom);
    for (int i=langCodeAll.size()-1; i>=0; --i) {
        QString langCode = langCodeAll[i];
        if (langWithcountries.contains(langCode)) {
            auto langCountries = langWithcountries.values(langCode);
            for (auto it = langCountries.begin(); it != langCountries.end(); ++it) {
                m_langsTo.insert(i, *it);
            }
        }
    }
    for (int i=0; i<m_langsTo.size(); ++i) {
        QString localeName = m_langsTo[i];
        if (localeName.contains("-")) {
            QString langCode = localeName.left(2).toLower();
            m_langCodeToOtherCountryColIndex.insert(langCode, i + 1);
        }
    }
}
//----------------------------------------
QString TranslationLinkManager::getLink(
        const QString &link, const QString &langTo) const
{
    if (langTo != m_langFrom && m_links.contains(link)) {
        int index = m_links[link];
        static QHash<QString, int> langToIndex
                = [this]() -> QHash<QString, int> {
            QHash<QString, int> indexes;
            indexes[m_langFrom] = 0;
            for (int i=0; i<m_langsTo.size(); ++i) {
                indexes[m_langsTo[i]] = i+1;
            }
            return indexes;
        }();
         // TODO factorize ?
        int indexColumn = langToIndex[langTo];
        return m_tableToTranslate[index][indexColumn];
    }
    return link;
}
//----------------------------------------
QHash<QString, QString> TranslationLinkManager::getAltLinksByCountryCode(
        const QString &link, const QString &langTo) const
{
    QHash<QString, QString> altLinks;
    //if (m_links.contains(link) && m_langCodeToOtherCountryColIndex.contains(langTo)) {
    if (m_langCodeToOtherCountryColIndex.contains(langTo)) {
        auto colIndexes = m_langCodeToOtherCountryColIndex.values(langTo);
        for (auto itColInd = colIndexes.begin(); itColInd != colIndexes.end(); ++itColInd) {
            QString colName = headerData(*itColInd, Qt::Horizontal).toString();
            if (m_links.contains(link)) {
                int rowIndex = m_links[link];
                altLinks[colName] = m_tableToTranslate[rowIndex][*itColInd];
            }
        }
    }
    return altLinks;
}
//----------------------------------------
QVariant TranslationLinkManager::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList header = QStringList(m_langFrom) << m_langsTo;
        return header[section];
    }
    return QVariant();
}
//----------------------------------------
int TranslationLinkManager::rowCount(const QModelIndex &) const
{
    return m_tableToTranslate.size();
}
//----------------------------------------
int TranslationLinkManager::columnCount(const QModelIndex &) const
{
    return m_langsTo.size() + 1;
}
//----------------------------------------
QVariant TranslationLinkManager::data(
        const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_tableToTranslate[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------
bool TranslationLinkManager::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole
            && data(index, Qt::DisplayRole) != value) {
        QString string = value.toString();
        m_tableToTranslate[index.row()][index.column()]
                = value.toString();
        save();
        return true;
    }
    return false;
}
//----------------------------------------
Qt::ItemFlags TranslationLinkManager::flags(const QModelIndex &index) const
{
    if (index.column() > 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------
void TranslationLinkManager::addLinksToTranslate(
        const QSet<QString> &links)
{
    for (auto link = links.begin(); link != links.end(); ++link) {
        if (!m_links.contains(*link)) {
            beginInsertRows(QModelIndex(), 0, 0);
            m_tableToTranslate.append(QStringList(columnCount()));
            m_tableToTranslate[m_tableToTranslate.size()-1][0]
                    = *link;
            m_links.insert(*link, m_tableToTranslate.size()-1);
            endInsertRows();
        }
    }
}
//----------------------------------------
void TranslationLinkManager::load()
{
    if (m_tableToTranslate.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        m_tableToTranslate.clear();
        m_links.clear();
        endRemoveRows();
    }

    int nCols = columnCount();
    QStringList colNames;
    for (int i=0; i<nCols; ++i) {
        QString colName = headerData(i, Qt::Horizontal).toString();
        colNames << colName;
    }

    QString filePath = m_workingDir.filePath(FILE_NAME_TRANSLATION);
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        QStringList lines = stream.readAll().split("\n");
        QStringList prevColNames = lines.takeFirst().split("\t");
        QHash<QString, int> colNameToIndex;
        for (int i=0; i<prevColNames.size(); ++i) {
            colNameToIndex[prevColNames[i]] = i;
        }
        int indLine = 0;
        for (auto itLine = lines.begin(); itLine != lines.end(); ++itLine) {
            QStringList elements(nCols);
            if (!itLine->trimmed().isEmpty()) {
                QStringList loadedElements = itLine->split("\t");;
                for (int i=0; i<colNames.size(); ++i) {
                    QString colName = colNames[i];
                    if (colNameToIndex.contains(colName)) {
                        elements[i] = loadedElements[colNameToIndex[colName]];
                    }
                }
                m_links[elements[0]] = indLine;
                m_tableToTranslate << elements;
            }
            ++indLine;
        }
        file.close();
    }
    if (m_tableToTranslate.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        endInsertRows();
    }
}
//----------------------------------------
void TranslationLinkManager::save()
{
    int nCols = columnCount();
    QStringList colNames;
    for (int i=0; i<nCols; ++i) {
        QString colName = headerData(i, Qt::Horizontal).toString();
        colNames << colName;
    }
    QStringList lines;
    lines << colNames.join("\t");
    for (auto it = m_tableToTranslate.begin();
         it != m_tableToTranslate.end(); ++it) {
        lines << it->join("\t");
    }
    QString filePath = m_workingDir.filePath(FILE_NAME_TRANSLATION);
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << lines.join("\n");
        file.close();
    }
}
//----------------------------------------
