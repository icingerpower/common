#include <QApplication>
#include <QMessageBox>
#include <QClipboard>

#include "TranslateTableModel.h"

#include "DialogPreTranslationCheck.h"
#include "ui_DialogPreTranslationCheck.h"

DialogPreTranslationCheck::DialogPreTranslationCheck(
        TranslateTableModel *translationTableModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPreTranslationCheck)
{
    ui->setupUi(this);
    int nRows = 10;
    ui->tableWidgetTrans->setRowCount(nRows);
    for (int i=0; i<nRows; ++i)
    {
        QString text = translationTableModel->index(i, 0).data().toString();
        ui->tableWidgetTrans->setItem(i, 0, new QTableWidgetItem{text});
    }
    _connectSlots();
}

DialogPreTranslationCheck::~DialogPreTranslationCheck()
{
    delete ui;
}

void DialogPreTranslationCheck::paste()
{
    auto clipboard = QApplication::clipboard();
    clipboard->text();
    QString text = clipboard->text();
    text.replace("\r\n", "\n");
    text.replace("\n\n", "\n");
    const QStringList &lines = text.split("\n");
    int nRows = qMin(ui->tableWidgetTrans->rowCount(), lines.size());
    for (int i=nRows; i<ui->tableWidgetTrans->rowCount(); ++i)
    {
        delete ui->tableWidgetTrans->takeItem(1, i);
    }
    for (int i=0; i<nRows; ++i)
    {
        const QString &textOrig = ui->tableWidgetTrans->item(i, 0)->text();
        auto newItem =  new QTableWidgetItem{lines[i]};
        if (lines[i] == textOrig)
        {
            newItem->setBackground(Qt::darkGreen);
        }
        else
        {
            newItem->setBackground(Qt::darkRed);
        }
        ui->tableWidgetTrans->setItem(i, 1, newItem);
    }
}

void DialogPreTranslationCheck::accept()
{
    for (int i=0; i<ui->tableWidgetTrans->rowCount(); ++i)
    {
        const QString &textOrig = ui->tableWidgetTrans->item(i, 0)->text();
        const QString &textLoaded = ui->tableWidgetTrans->item(i, 1)->text();
        if (textOrig.compare(textLoaded, Qt::CaseInsensitive) != 0)
        {
            QMessageBox::warning(this,
                                 tr("Text doesn’t match"),
                                 tr("The loaded text doesn’t match the text to translate"));
            return;
        }
    }
    QDialog::accept();
}

void DialogPreTranslationCheck::_connectSlots()
{
    connect(ui->buttonUpload,
            &QPushButton::clicked,
            this,
            &DialogPreTranslationCheck::uploadAsked);
    connect(ui->buttonPaste,
            &QPushButton::clicked,
            this,
            &DialogPreTranslationCheck::paste);
}
