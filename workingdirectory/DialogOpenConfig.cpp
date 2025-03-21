#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>

#include "DialogOpenConfig.h"
#include "ui_DialogOpenConfig.h"

#include "WorkingDirectoryManager.h"

//----------------------------------------
DialogOpenConfig::DialogOpenConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOpenConifg)
{
    ui->setupUi(this);
    m_rejected = false;
    m_opening = false;
    _connectSlots();
    QStringList recentlyOpen = WorkingDirectoryManager::instance()->recentlyOpen();
    if (recentlyOpen.size() > 0)
    {
        ui->labelConfigInfo->hide();
        ui->listRecentPaths->addItems(recentlyOpen);
        ui->buttonOpenRecent->setFocus();
        ui->listRecentPaths->setCurrentRow(0);
    }
    else
    {
        ui->buttonOpenRecent->hide();
        ui->buttonClearRecentSelected->hide();
        ui->labelRecentlyOpened->hide();
    }
}
//----------------------------------------
void DialogOpenConfig::_connectSlots()
{
    connect(ui->buttonOpenRecent,
            &QPushButton::clicked,
            this,
            &DialogOpenConfig::openRecent);
    connect(ui->buttonClearRecentSelected,
            &QPushButton::clicked,
            this,
            &DialogOpenConfig::clearRecentSelected);
    connect(ui->buttonBrowseAndOpen,
            &QPushButton::clicked,
            this,
            &DialogOpenConfig::browseAndOpen);
}
//----------------------------------------
DialogOpenConfig::~DialogOpenConfig()
{
    delete ui;
}
//----------------------------------------
bool DialogOpenConfig::wasRejected() const
{
    return m_rejected;
}
//----------------------------------------
bool DialogOpenConfig::wasAccepted() const
{
    return !m_rejected;
}
//----------------------------------------
void DialogOpenConfig::clearRecentSelected()
{
    auto selectedItems = ui->listRecentPaths->selectedItems();
    if (selectedItems.size() == 1)
    {
        int row = ui->listRecentPaths->currentRow();
        QString recent = selectedItems.first()->data(
                    Qt::DisplayRole).toString();
        WorkingDirectoryManager::instance()->removeRecent(recent);
        ui->listRecentPaths->takeItem(row);
    }
}
//----------------------------------------
void DialogOpenConfig::openRecent()
{
    auto selectedItems = ui->listRecentPaths->selectedItems();
    if (selectedItems.size() == 1)
    {
        QString path = selectedItems[0]->text();
        if (!QDir(path).exists())
        {
            QMessageBox::warning(
                        this,
                        tr("Directory not found"),
                        tr("The selected directory doesn’t exist anymore."));
        }
        else
        {
            WorkingDirectoryManager::instance()->open(path);
            openAndAccept();
        }
    }
    else
    {
        QMessageBox::warning(
                    this,
                    tr("Selection", "Not one directory selected"),
                    tr("You need to select one recent directory in the list."));
    }
}
//----------------------------------------
void DialogOpenConfig::browseAndOpen()
{
    QSettings settings;
    QString settingKey = "DialogOpenConifg::browseAndOpen";
    QString lastDirPath = settings.value(
                settingKey, QDir().absolutePath()).toString();
    QString dirPath = QFileDialog::getExistingDirectory(
                this,
                tr("Chose a directory"),
                lastDirPath,
                QFileDialog::DontUseNativeDialog);
    if (!dirPath.isEmpty())
    {
        QDir dir(dirPath);
        const QStringList &fileNames = dir.entryList(QDir::Files);
        bool dirEmpty = fileNames.size() == 0;
        bool dirWithValidSettings = fileNames.contains(
                    WorkingDirectoryManager::instance()->settingsFileName());
        if (!dirEmpty && !dirWithValidSettings)
        {
            QMessageBox::StandardButton reply
                    = QMessageBox::question(
                        this,
                        tr("Working folder not empty"),
                        tr("We recommend using a new empty folder. Are you sure that you want to use the selected folder that already contains files?"),
                        QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No)
            {
                return;
            }
        }
        settings.setValue(settingKey, QFileInfo(dirPath).path());
        WorkingDirectoryManager::instance()->open(dirPath);
        openAndAccept();
    }
}
//----------------------------------------
void DialogOpenConfig::openAndAccept()
{
    m_opening = true;
    accept();
}
//----------------------------------------
void DialogOpenConfig::accept()
{
    if (m_opening)
    {
        QDialog::accept();
    }
}
//----------------------------------------
void DialogOpenConfig::reject()
{
    m_rejected = true;
    QDialog::reject();
}
//----------------------------------------
