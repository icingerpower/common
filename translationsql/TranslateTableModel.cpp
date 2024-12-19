#include <QSettings>

#include "ExceptionTranslation.h"

#include "TranslateTableModel.h"

//----------------------------------------------------------
const int TranslateTableModel::PERCENTAGE_TRANS_SAME{50};
const int TranslateTableModel::N_PREVIOUS_TRANS_SIGNATURE{20};
const QString TranslateTableModel::KEY_PREVIOUS_TRANS{"previousTransSignature"};
const QString TranslateTableModel::KEY_TEMPORARY_TRANSLATION{"TempTranslation"};
const QString TranslateTableModel::KEY_TEMPORARY_TRANSLATION_BACKUP{"TempTranslationBackup"};
const QString TranslateTableModel::KEY_TEMPORARY_HEADER{"TempTranslationHeader"};
const QString TranslateTableModel::KEY_TEMPORARY_HEADER_BACKUP{"TempTranslationBackupHeader"};
const QString TranslateTableModel::KEY_TEMPORARY_LANGS{"TempTranslationLangs"};
const QString TranslateTableModel::KEY_TEMPORARY_LANGS_BACKUP{"TempTranslationLangsBackup"};
//----------------------------------------------------------
TranslateTableModel::TranslateTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}
//----------------------------------------------------------
TranslateTableModel::TranslateTableModel(
        const QStringList &texts, const QStringList &langs, QObject *parent)
    : QAbstractTableModel(parent)
{
    _initHeader(langs);
    QStringList lineValues{m_header.size(), QString{}};
    for (const auto &text : texts)
    {
        lineValues[0] = text;
        m_listOfStringList << lineValues;
    }
}
//----------------------------------------------------------
void TranslateTableModel::_initHeader(const QStringList &langs)
{
    m_header << tr("Keyword");
    m_langs = langs;
    for (const auto &lang : langs)
    {
        m_header << lang;
    }
}
//----------------------------------------------------------
TranslateTableModel::TranslateTableModel(
        const QSet<QStringView> &texts, const QStringList &langs, QObject *parent)
{
    _initHeader(langs);
    QStringList lineValues{m_header.size(), QString{}};
    for (const auto &text : texts)
    {
        lineValues[0] = text.toString();
        m_listOfStringList << lineValues;
    }
}
//----------------------------------------------------------
bool TranslateTableModel::areAllTranslationsDone() const
{
    if (m_listOfStringList.size() > 0)
    {
        for (int i=1; i<columnCount(); ++i)
        {
            if (m_listOfStringList[0][i].isEmpty())
            {
                return false;
            }
        }
    }
    return true;
}
//----------------------------------------------------------
QStringList TranslateTableModel::getLangCodes() const
{
    return m_langs;
}
//----------------------------------------------------------
void TranslateTableModel::pasteTranslatedText(
        const QModelIndex &index, const QStringList &lines, bool force)
{
    const QString &langTo = m_header[index.column()].toLower();
    QSet<QString> countriesWithEnglish{"se", "pl", "nl", "sg"};
    int nSameMax = 17;
    int percentageSameMax = PERCENTAGE_TRANS_SAME;
    if (countriesWithEnglish.contains(langTo))
    {
        nSameMax = 30;
        percentageSameMax = 85;
    }
    if (QFile::exists("transparams.ini"))
    {
        QSettings settings("transparams.ini", QSettings::IniFormat);
        nSameMax = settings.value("nSameMax", nSameMax).toInt();
        percentageSameMax = settings.value("percentageSameMax", percentageSameMax).toInt();
    }
    if (index.column() == 0)
    {
        ExceptionTranslation exception;
        exception.setTitle(tr("Wrong column"));
        exception.setError(
                    tr("The first column can’t be modified"));
        exception.raise();
    }
    else
    {
        if (lines.size() != m_listOfStringList.size())
        {
            ExceptionTranslation exception;
            exception.setTitle(tr("Wrong size"));
            exception.setError(
                        "The size of the pasted lines is "
                        + QString::number(lines.size())
                        + " instead of "
                        + QString::number(m_listOfStringList.size()));
            exception.raise();
        }
        if (!force)
        {
            // Checking if text is same more than 10 times
            int nSame = 0;
            for (int i=0; i<lines.size(); ++i)
            {
                if (m_listOfStringList[i][0] == lines[i])
                {
                    ++nSame;
                }
                else
                {
                    nSame = 0;
                }
                if (nSame == nSameMax)
                {
                    ExceptionTranslation exception;
                    exception.setTitle(tr("Missing translations"));
                    exception.setError(
                                "Starting line "
                                + QString::number(i+1)
                                + " (" + lines[i]
                                + ") the lines are the same");
                    exception.raise();
                }
            }
            // TODO check translation is not same as a previous translation
            const auto &sameAsPrevious = _countNumberSameAsPrevious(langTo, lines);
            if (sameAsPrevious.size() > N_PREVIOUS_TRANS_SIGNATURE * PERCENTAGE_TRANS_SAME / 100.)
            {
                    ExceptionTranslation exception;
                    exception.setTitle("Translations are the same as before");
                    QString error = "The translations seem to be the same as before. Check that the website was well refrshed.";
                    for (const auto &previous : sameAsPrevious)
                    {
                        error += QString{"\n"};
                        error += previous.join(" / ");
                    }
                    exception.setError(error);
                    exception.raise();
            }
            // TODO check translation is not same with another column
            int countSameAsOtherColumn = _countNumberSameAsAlreadyPasted(lines);
            if (countSameAsOtherColumn > lines.size() * percentageSameMax / 100.)
            {
                ExceptionTranslation exception;
                exception.setTitle("Translations are the same as another column");
                QString error = "The translations seem to be the same as another column. Please double check.";
                exception.setError(error);
                exception.raise();
            }
        }
        for (int i=0; i<lines.size(); ++i)
        {
            m_listOfStringList[i][index.column()] = lines[i];
        }
        _saveCurrentSignature(langTo, lines);
        emit dataChanged(index.siblingAtRow(0),
                         index.siblingAtRow(rowCount()-1));
    }
}
//----------------------------------------------------------
QStringList TranslateTableModel::getWordsToTranslate() const
{
    QStringList translations;
    for (const auto &stringList : m_listOfStringList)
    {
        translations << stringList[0];
    }
    return translations;
}
//----------------------------------------------------------
QStringList TranslateTableModel::getTranslations(const QString &lang) const
{
    QStringList translations;
    int colIndex = m_langs.indexOf(lang);
    if (colIndex > -1)
    {
        ++colIndex;
        for (int i=0; i<m_listOfStringList.size(); ++i)
        {
            translations << m_listOfStringList[i][colIndex];
        }
    }
    return translations;
}
//----------------------------------------------------------
QVariant TranslateTableModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            return m_header[section];
        }
        else if (orientation == Qt::Vertical)
        {
            return QString::number(section + 1);
        }
    }
    return QVariant{};
}
//----------------------------------------------------------
int TranslateTableModel::rowCount(const QModelIndex &) const
{
    return m_listOfStringList.size();
}
//----------------------------------------------------------
int TranslateTableModel::columnCount(const QModelIndex &) const
{
    return m_header.size();
}
//----------------------------------------------------------
QVariant TranslateTableModel::data(
        const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            return m_listOfStringList[index.row()][index.column()];
        }
    }
    return QVariant{};
}
//----------------------------------------------------------
bool TranslateTableModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && value != data(index))
    {
        m_listOfStringList[index.row()][index.column()] = value.toString();
        emit dataChanged(index, index, QList<int>{Qt::DisplayRole});
        return true;
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags TranslateTableModel::flags(
        const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}
//----------------------------------------------------------
void TranslateTableModel::saveTemporary() const
{
    QSettings settings;
    settings.setValue(KEY_TEMPORARY_TRANSLATION,
                      QVariant::fromValue(m_listOfStringList));
    settings.setValue(KEY_TEMPORARY_HEADER,
                      m_header);
    settings.setValue(KEY_TEMPORARY_LANGS,
                      m_langs);
}
//----------------------------------------------------------
void TranslateTableModel::saveBackup() const
{
    QSettings settings;
    settings.setValue(KEY_TEMPORARY_TRANSLATION_BACKUP,
                      QVariant::fromValue(m_listOfStringList));
    settings.setValue(KEY_TEMPORARY_HEADER_BACKUP,
                      m_header);
    settings.setValue(KEY_TEMPORARY_LANGS_BACKUP,
                      m_langs);
}
//----------------------------------------------------------
void TranslateTableModel::clearTemporary()
{
    QSettings settings;
    settings.remove(KEY_TEMPORARY_TRANSLATION);
    settings.remove(KEY_TEMPORARY_HEADER);
    settings.remove(KEY_TEMPORARY_LANGS);
}
//----------------------------------------------------------
void TranslateTableModel::loadTemporary()
{
    _loadTemporary(KEY_TEMPORARY_TRANSLATION,
                   KEY_TEMPORARY_HEADER,
                   KEY_TEMPORARY_LANGS);
}
//----------------------------------------------------------
void TranslateTableModel::_loadTemporary(
        const QString &settingKey,
        const QString &settingKeyHeader,
        const QString &settingKeyLang)
{
    _clear();
    QSettings settings;
    auto listOfStringList = settings.value(settingKey).value<QList<QStringList>>();
    if (listOfStringList.size() > 0)
    {
        beginInsertRows(QModelIndex(), 0, listOfStringList.size()-1);
        m_listOfStringList = std::move(listOfStringList);
        m_header = settings.value(settingKeyHeader).toStringList();
        m_langs = settings.value(settingKeyLang).toStringList();
        endInsertRows();
    }
}
//----------------------------------------------------------
void TranslateTableModel::_clear()
{
    if (m_listOfStringList.size() > 0)
    {
        beginRemoveRows(QModelIndex{}, 0, m_listOfStringList.size()-1);
        m_listOfStringList.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
QString TranslateTableModel::_settingsKeyPreviousTrans(
        const QString &langTo) const
{
    return KEY_PREVIOUS_TRANS + langTo;
}
//----------------------------------------------------------
QList<QStringList> TranslateTableModel::_getPreviousTransSignature(
        const QString &langTo) const
{
    const QString &key = _settingsKeyPreviousTrans(langTo);
    QSettings settings;
    if (settings.contains(key))
    {
        return settings.value(key).value<QList<QStringList>>();
    }
    return QList<QStringList>{};
}
//----------------------------------------------------------
void TranslateTableModel::_saveCurrentSignature(
        const QString &langTo, const QStringList &keywordsTranslated) const
{
    QList<QStringList> curSignature;
    int nSignature = qMin(N_PREVIOUS_TRANS_SIGNATURE, keywordsTranslated.size());
    for (int i=0; i<nSignature; ++i)
    {
        curSignature << QStringList{m_listOfStringList[i][0], keywordsTranslated[i]};
    }
    const QString &key = _settingsKeyPreviousTrans(langTo);
    QSettings settings;
    settings.setValue(key, QVariant::fromValue(curSignature));
}
//----------------------------------------------------------
QList<QStringList> TranslateTableModel::_countNumberSameAsPrevious(
        const QString &langTo, const QStringList &keywordsTranslated) const
{
    QList<QStringList> sameTranslations;
    const auto &previousSignature = _getPreviousTransSignature(langTo);
    if (previousSignature.size() > 0)
    {
        QList<QStringList> curSignature;
        int nSignature = qMin(N_PREVIOUS_TRANS_SIGNATURE, keywordsTranslated.size());
        nSignature = qMin(nSignature, previousSignature.size());
        for (int i=0; i<nSignature; ++i)
        {
            curSignature << QStringList{m_listOfStringList[i][0], keywordsTranslated[i]};
            if (curSignature.last()[0] != previousSignature[i][0]
                    && curSignature.last()[1] == previousSignature[i][1])
            {
                sameTranslations << QStringList{
                                    curSignature.last()[0]
                                    , curSignature.last()[1]
                                    , previousSignature[i][0]
                                    , previousSignature[i][1]};
            }
        }
    }
return sameTranslations;
}
//----------------------------------------------------------
//----------------------------------------------------------
int TranslateTableModel::_countNumberSameAsAlreadyPasted(
        const QStringList &keywordsTranslated) const
{
    int nSame = 0;
    for (int i=0; i<columnCount(); ++i)
    {
        if (!m_listOfStringList[0][i].isEmpty())
        {
            int nSameCurrent = 0;
            for (int j=0; j<m_listOfStringList.size(); ++j)
            {
                if (m_listOfStringList[j][i] == keywordsTranslated[j])
                {
                    ++nSameCurrent;
                }
            }
            nSame = qMax(nSameCurrent, nSame);
        }
    }
    return nSame;
}
//----------------------------------------------------------
void TranslateTableModel::loadBackupAfterCrash()
{
    _loadTemporary(KEY_TEMPORARY_TRANSLATION_BACKUP,
                   KEY_TEMPORARY_HEADER_BACKUP,
                   KEY_TEMPORARY_LANGS_BACKUP);
}
//----------------------------------------------------------


