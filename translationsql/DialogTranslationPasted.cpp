#include <QTableWidgetItem>
#include <QClipboard>

#include "TranslateTableModel.h"

#include "DialogTranslationPasted.h"
#include "ui_DialogTranslationPasted.h"

DialogTranslationPasted::DialogTranslationPasted(
        TranslateTableModel *translateTableModel,
        const QString &imageFilePath,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTranslationPasted)
{
    ui->setupUi(this);
    const QStringList &header = translateTableModel->header();
    auto weirdTranslations = translateTableModel->getWeirdTranslations();
    int nRows = weirdTranslations.size();
    int nColumns = translateTableModel->columnCount();
    ui->tableWidgetKeywords->setColumnCount(nColumns);
    ui->tableWidgetKeywords->setRowCount(nRows);
    ui->tableWidgetKeywords->setHorizontalHeaderLabels(header);
    for (int i=0; i<nRows; ++i)
    {
        for (int j=0; j<nColumns; ++j)
        {
            ui->tableWidgetKeywords->setItem(
                        i, j, new QTableWidgetItem{
                            weirdTranslations[i][j]});
        }
    }
    auto translations = translateTableModel->pickTranslationsToCheck(
                3);
    translations += translateTableModel->pickTranslationsToCheck(
                3, weirdTranslations);
    ui->plainTextPrompt->setPlainText(
                translateTableModel->createChatGptPromptCheck(translations));
    connect(ui->buttonCopy,
            &QPushButton::clicked,
            this,
            &DialogTranslationPasted::copyPrompt);
}

DialogTranslationPasted::~DialogTranslationPasted()
{
    delete ui;
}

QString DialogTranslationPasted::getPrompt() const
{
    return ui->plainTextPrompt->toPlainText();
}

void DialogTranslationPasted::copyPrompt()
{
    auto clipboard = QApplication::clipboard();
    clipboard->setText(ui->plainTextPrompt->toPlainText());
}

void DialogTranslationPasted::accept()
{
    auto pixmap = grab();
    pixmap.save(m_imageFilePath);
    QDialog::accept();
}
