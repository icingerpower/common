#include <QtWidgets/qcombobox.h>

#include "../../countries/CountryManager.h"

#include "DelegateCountries.h"

//----------------------------------------------------------
DelegateCountries::DelegateCountries(
        std::function<bool (const QModelIndex &)> indexValid,
        QObject *parent)
    : QStyledItemDelegate(parent)
{
    m_indexValid = indexValid;
    m_ueOnly = false;
}
//----------------------------------------------------------
QWidget *DelegateCountries::createEditor(
        QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *widget = nullptr;
    if (m_indexValid(index)) {
        QComboBox *comboBox = new QComboBox(parent);
        if (m_ueOnly) {
            comboBox->addItems(*CountryManager::countriesNamesUEfrom2020());
        } else {
            comboBox->addItems(*CountryManager::countryNames());
        }
        widget = comboBox;
    } else {
        widget =  QStyledItemDelegate::createEditor(
                    parent, option, index);
    }
    return widget;
}
//----------------------------------------------------------
void DelegateCountries::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (m_indexValid(index)) {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        QString countryName = index.data().toString();
        //QString country = CountryManager::countryName(countryCode);
        comboBox->setCurrentText(countryName);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
//----------------------------------------------------------
bool DelegateCountries::ueOnly() const
{
    return m_ueOnly;
}
//----------------------------------------------------------
void DelegateCountries::setUeOnly(bool ueOnly)
{
    m_ueOnly = ueOnly;
}
//----------------------------------------------------------
