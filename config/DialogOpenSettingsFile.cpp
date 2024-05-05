#include <QFileDialog>
#include <QMessageBox>

#include "SettingsManager.h"

#include "DialogOpenSettingsFile.h"
#include "ui_DialogOpenSettingsFile.h"

DialogOpenSettingsFile::DialogOpenSettingsFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOpenSettingsFile)
{
    ui->setupUi(this);
    ui->listViewPreviousSettingsFiles->setModel(
                SettingsManager::instance());
    m_wasAccepted = false;
    connect(ui->buttonOpenNew,
            &QPushButton::clicked,
            this,
            &DialogOpenSettingsFile::createSettingsFile);
    connect(ui->buttonOpenExisting,
            &QPushButton::clicked,
            this,
            &DialogOpenSettingsFile::browseSettingsFile);
    connect(ui->listViewPreviousSettingsFiles->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            [this](){
        ui->lineEditSettingsFile->clear();
    });
}

DialogOpenSettingsFile::~DialogOpenSettingsFile()
{
    delete ui;
}

bool DialogOpenSettingsFile::wasAccepted()
{
    return m_wasAccepted;
}

QString DialogOpenSettingsFile::getFilePath() const
{
    if (ui->lineEditSettingsFile->text().isEmpty())
    {
        const auto &selIndexes
                = ui->listViewPreviousSettingsFiles->selectionModel()->selectedIndexes();
        if (selIndexes.size() > 0)
        {
            return selIndexes.first().data().toString();
        }
    }
    else
    {
        return ui->lineEditSettingsFile->text();
    }
    return QString{};
}

void DialogOpenSettingsFile::createSettingsFile()
{
    const auto &filePath = QFileDialog::getSaveFileName(
                this,
                "Settings file",
                QString{},
                QString{"INI (*.ini)"});
    if (!filePath.isEmpty())
    {
        ui->listViewPreviousSettingsFiles->clearSelection();
        ui->lineEditSettingsFile->setText(filePath);
    }
}

void DialogOpenSettingsFile::browseSettingsFile()
{
    const auto &filePath = QFileDialog::getOpenFileName(
                this,
                "Settings file",
                QString{},
                QString{"INI (*.ini)"});
    if (!filePath.isEmpty())
    {
        ui->listViewPreviousSettingsFiles->clearSelection();
        ui->lineEditSettingsFile->setText(filePath);
    }
}

void DialogOpenSettingsFile::accept()
{
    if (getFilePath().isEmpty())
    {
        QMessageBox::information(
                    this,
                    "No file selected",
                    "You need to select a settings file");
    }
    else
    {
        m_wasAccepted = true;
        QDialog::accept();
    }
}

void DialogOpenSettingsFile::reject()
{
    m_wasAccepted = false;
    QDialog::reject();
}
