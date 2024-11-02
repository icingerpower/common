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
    ui->listRecentPaths->addItems(recentlyOpen);
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
    if (selectedItems.size() == 1) {
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
    if (selectedItems.size() == 1) {
        QString path = selectedItems[0]->text();
        if (!QDir(path).exists()) {
            QMessageBox::warning(
                        this,
                        tr("Directory not found"),
                        tr("The selected directory doesn’t exist anymore."));
        } else {
            WorkingDirectoryManager::instance()->open(path);
            openAndAccept();
        }
    } else {
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
                lastDirPath);
    if (!dirPath.isEmpty()) {
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
    if (m_opening) {
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
