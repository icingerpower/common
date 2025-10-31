#include <QSettings>
#include <QLocale>

#include "LangManager.h"

const QHash<QString, QString> LangManager::COUNTRY_TO_LANG{
    {"com","en"}
    , {"uk","en"}
    , {"ca","en"}
    , {"au","en"}
    , {"sg","en"}
    , {"mx","es"}
    , {"es","es"}
    , {"jp","ja"}
    , {"de","de"}
    , {"fr","fr"}
    , {"it","it"}
    , {"es","es"}
    , {"se","se"}
    , {"nl","nl"}
    , {"in","in"}
    , {"tr","tr"}
    , {"sa","ar"}
    , {"eg","ar"}
    , {"ae","ar"}
};

LangManager::LangManager(QObject *parent)
    : QAbstractTableModel(parent)
{
    QStringList langCodes{LANGUE_CODES_JOINED};
    for (const auto &langCode : langCodes)
    {
        auto language = QLocale::codeToLanguage(langCode);
        const auto &languageName = QLocale::languageToString(language);
        m_listOfStringList << QStringList{langCode.toUpper(), languageName};
    }
}

bool LangManager::hasTranslation(const QString &langCode) const
{
    for (const auto &stringList : m_listOfStringList)
    {
        if (stringList.first().toLower() == langCode.toLower())
        {
            return true;
        }
    }
    return false;
}

QStringList LangManager::langCodesFrom(const QSet<QString> &countryCodesFrom) const
{
    QStringList langCodes;
    for (const auto &countryCodeFrom : countryCodesFrom)
    {
        Q_ASSERT(LangManager::COUNTRY_TO_LANG.contains(countryCodeFrom));
        const QString &langCodeFrom = LangManager::COUNTRY_TO_LANG[countryCodeFrom];
        langCodes << langCodeFrom;
    }
    return langCodes;
}

QStringList LangManager::langCodesTo() const
{
    QStringList langCodes;
    for (const auto &stringList : m_listOfStringList)
    {
        langCodes << stringList.first().toLower();
    }
    return langCodes;
}

QVariant LangManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            static QStringList header{tr("Code"), tr("Language")};
            return header[section];
        }
        else if (orientation == Qt::Vertical)
        {
            return QString::number(section + 1);
        }
    }
    return QVariant{};
}

bool LangManager::containsLang(const QString &langCode) const
{
    for (const auto &stringList : m_listOfStringList)
    {
        if (stringList[0].toLower() == langCode.toLower())
        {
            return true;
        }
    }
    return false;
}

int LangManager::rowCount(const QModelIndex &) const
{
    return m_listOfStringList.size();
}

int LangManager::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant LangManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_listOfStringList[index.row()][index.column()];
    }
    return QVariant();
}

