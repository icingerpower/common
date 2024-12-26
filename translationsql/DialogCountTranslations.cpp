#include "TrSqlManager.h"

#include "DialogCountTranslations.h"
#include "ui_DialogCountTranslations.h"

DialogCountTranslations::DialogCountTranslations(
        TrSqlManager *trSqlManager,
        const QStringList &langCodesFrom,
        const QStringList &langCodesTo,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCountTranslations)
{
    ui->setupUi(this);
    int i=0;
    ui->tableWidgetCount->setRowCount(langCodesFrom.size() + 1);
    unsigned long total = 0;
    unsigned long totalRows = 0;
    for (const auto &langCode : langCodesFrom)
    {
        unsigned long nRows = trSqlManager->countRows(langCode);
        unsigned long nLangs = langCodesTo.size();
        if (langCodesTo.contains(langCode))
        {
            --nLangs;
        }
        unsigned long langTotal = nRows * nLangs;
        totalRows += nRows;

        QTableWidgetItem *itemLangCode = new QTableWidgetItem{langCode};
        itemLangCode->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidgetCount->setItem(i, 0, itemLangCode);

        QTableWidgetItem *itemNRows = new QTableWidgetItem{QString::number(nRows)};
        itemNRows->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidgetCount->setItem(i, 1, itemNRows);

        QTableWidgetItem *itemNLangs = new QTableWidgetItem{QString::number(nLangs)};
        itemNLangs->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidgetCount->setItem(i, 2, itemNLangs);

        QTableWidgetItem *itemLangTotal = new QTableWidgetItem{QString::number(langTotal)};
        itemLangTotal->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidgetCount->setItem(i, 3, itemLangTotal);

        total += langTotal;
        ++i;
    }

    QTableWidgetItem *itemTotalLabel = new QTableWidgetItem{tr("Total")};
    itemTotalLabel->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidgetCount->setItem(i, 0, itemTotalLabel);

    QTableWidgetItem *itemTotalRows = new QTableWidgetItem{QString::number(totalRows)};
    itemTotalRows->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidgetCount->setItem(i, 1, itemTotalRows);

    QTableWidgetItem *itemTotal = new QTableWidgetItem{QString::number(total)};
    itemTotal->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidgetCount->setItem(i, 3, itemTotal);
}

DialogCountTranslations::~DialogCountTranslations()
{
    delete ui;
}
