#include "ExceptionTranslation.h"

#include "TranslateTableModel.h"

//----------------------------------------------------------
TranslateTableModel::TranslateTableModel(
        const QStringList &texts, const QStringList &langs, QObject *parent)
    : QAbstractTableModel(parent)
{
    QStringList colValues{QString{}};
    m_header << tr("Keyword");
    for (const auto lang : langs)
    {
        colValues << QString{};
        m_header << lang;
    }
    for (const auto &text : texts)
    {
        colValues[0] = text;
        m_listOfStringList << colValues;
    }

}
//----------------------------------------------------------
void TranslateTableModel::pasteTranslatedText(
        const QModelIndex &index, const QStringList &lines, bool force)
{
    if (index.column() == 0)
    {
        ExceptionTranslation exception;
        exception.setError(
                    tr("The first column can’t be modified"));
        exception.raise();
    }
    else
    {
        if (lines.size() != m_listOfStringList.size())
        {
            ExceptionTranslation exception;
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
                if (m_listOfStringList[i][index.column()] == lines[i])
                {
                    ++nSame;
                }
                else
                {
                    nSame = 0;
                }
                if (nSame == 10)
                {
                    ExceptionTranslation exception;
                    exception.setError(
                                "Starting line "
                                + QString::number(i+1)
                                + " (" + lines[i]
                                + ") the lines are the same");
                    exception.raise();
                }
            }
        }
        for (int i=0; i<lines.size(); ++i)
        {
            m_listOfStringList[i][index.column()] = lines[i];
        }
        emit dataChanged(index.siblingAtRow(0),
                         index.siblingAtRow(rowCount()-1));
    }
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

