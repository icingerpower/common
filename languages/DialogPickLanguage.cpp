#include <QLocale>

#include "DialogPickLanguage.h"
#include "ui_DialogPickLanguage.h"

DialogPickLanguage::DialogPickLanguage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPickLanguage)
{
    ui->setupUi(this);
    _loadLangages();
    _connectSlots();
}

void DialogPickLanguage::_loadLangages()
{
    QSet<QString> codesDone;
    for (int i = QLocale::C + 1; i <= QLocale::LastLanguage; ++i) {
        QLocale::Language lang = static_cast<QLocale::Language>(i);
        const QString &code = QLocale::languageToCode(lang).toUpper();
        if (!code.isEmpty() && !codesDone.contains(code))
        {
            const QString &name = QLocale::languageToString(lang); // Get the full language name
            if (!name.isEmpty())
            {
                int nRows = ui->tableWidgetLangs->rowCount();;
                ui->tableWidgetLangs->setRowCount(nRows + 1);
                ui->tableWidgetLangs->setItem(nRows, 0, new QTableWidgetItem{code});
                ui->tableWidgetLangs->setItem(nRows, 1, new QTableWidgetItem{name});
                codesDone.insert(code);
            }
        }
    }
}

void DialogPickLanguage::_connectSlots()
{
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &DialogPickLanguage::filter);
    connect(ui->buttonFilterReset,
            &QPushButton::clicked,
            this,
            &DialogPickLanguage::filterReset);
}

DialogPickLanguage::~DialogPickLanguage()
{
    delete ui;
}

QString DialogPickLanguage::getLangCode() const
{
    const auto &selItems = ui->tableWidgetLangs->selectedItems();
    if (selItems.size() > 0)
    {
        return selItems.first()->text();
    }
    return QString{};
}

QString DialogPickLanguage::getLangName() const
{
    const auto &selItems = ui->tableWidgetLangs->selectedItems();
    if (selItems.size() > 0)
    {
        return selItems.last()->text();
    }
    return QString{};
}

void DialogPickLanguage::filter()
{
    ui->tableWidgetLangs->clearSelection();
    const QString &textFilter = ui->lineEditFilter->text();
    int nRows = ui->tableWidgetLangs->rowCount();
    for (int i=0; i<nRows; ++i)
    {
        bool visible = ui->tableWidgetLangs->item(i, 0)->text().contains(textFilter, Qt::CaseInsensitive)
                || ui->tableWidgetLangs->item(i, 1)->text().contains(textFilter, Qt::CaseInsensitive);
        ui->tableWidgetLangs->setRowHidden(i, !visible);
    }
}

void DialogPickLanguage::filterReset()
{
    int nRows = ui->tableWidgetLangs->rowCount();
    for (int i=0; i<nRows; ++i)
    {
        ui->tableWidgetLangs->setRowHidden(i, false);
    }
}
