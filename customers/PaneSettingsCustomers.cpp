#include <QInputDialog>
#include <QMessageBox>
#include <QStringListModel>

#include "CustomerTableModel.h"
#include "Customer.h"
#include "DialogEditCustomer.h"

#include "PaneSettingsCustomers.h"
#include "ui_PaneSettingsCustomers.h"

PaneSettingsCustomers::PaneSettingsCustomers(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneSettingsCustomers)
{
    ui->setupUi(this);
}

void PaneSettingsCustomers::_connectSlots()
{
    connect(ui->buttonAddCustomer,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::addCustomer);
    connect(ui->buttonRemoveCustomer,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::removeCustomer);
    connect(ui->buttonSearch,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::search);
    connect(ui->buttonSearchReset,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::searchReset);
    connect(ui->buttonAddComputerId,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::addComputerId);
    connect(ui->buttonRemoveComputerId,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::removeComputerId);
    connect(ui->buttonReplaceComputerId,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::replaceComputerId);
    connect(ui->tableViewCustomers->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PaneSettingsCustomers::selectCustomer);
}

PaneSettingsCustomers::~PaneSettingsCustomers()
{
    delete ui;
}

void PaneSettingsCustomers::init(
    const QString &settingsFilePath,
    const QString &settingsFilePathSecret)
{
    auto model = ui->tableViewCustomers->model();
    if (model != nullptr)
    {
        ui->tableViewCustomers->clearSelection();
        ui->tableViewCustomers->setModel(nullptr);
        model->deleteLater();
    }
    m_customerTableModel
        = new CustomerTableModel{settingsFilePath, settingsFilePathSecret};
    ui->tableViewCustomers->setModel(
        m_customerTableModel);
    ui->tableViewCustomers->setColumnWidth(1, 180);
    _connectSlots();
}

void PaneSettingsCustomers::addCustomer()
{
    DialogEditCustomer dialogCustomer;
    dialogCustomer.exec();
    if (dialogCustomer.wasAccepted())
    {
        auto customer = dialogCustomer.getCustomer();
        m_customerTableModel->addCustomer(customer);
    }
}

void PaneSettingsCustomers::removeCustomer()
{
    const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        m_customerTableModel->removeCustomer(selIndexes.first());
    }
}

void PaneSettingsCustomers::search()
{
    const QString &textFilter = ui->lineEditFilter->text();
    if (textFilter.isEmpty())
    {
        searchReset();
    }
    else
    {
        int nCols = m_customerTableModel->columnCount();
        int nRows = m_customerTableModel->rowCount();
        for (int i=0; i<nRows; ++i)
        {
            bool toHide = true;
            for (int j=0; j<nCols; ++j)
            {
                if (m_customerTableModel->index(i, j).data().toString().contains(textFilter, Qt::CaseInsensitive))
                {
                    toHide = false;
                    break;
                }
            }
            ui->tableViewCustomers->setRowHidden(i, toHide);
        }
    }
}

void PaneSettingsCustomers::searchReset()
{
    int nRows = m_customerTableModel->rowCount();
    for (int i=0; i<nRows; ++i)
    {
        ui->tableViewCustomers->setRowHidden(i, false);
    }
}

void PaneSettingsCustomers::addComputerId()
{
    const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        const QString &computerId = QInputDialog::getText(
                    this, tr("Computer ID"), tr("Enter the computer ID"));
        if (!computerId.isEmpty())
        {
            m_customerTableModel->addComputerId(
                        selIndexes.first(),
                        computerId);
        }
    }
    else
    {
        QMessageBox::information(
                    this,
                    tr("No customer"),
                    tr("You need to select a customer"));
    }
}

void PaneSettingsCustomers::replaceComputerId()
{
    const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        auto selComputerIds = ui->listViewIps->selectionModel()->selectedIndexes();
        if (selComputerIds.size() > 0)
        {
            const QString &newComputerId = QInputDialog::getText(
                        this, tr("Computer ID"), tr("Enter the computer ID"));
            if (!newComputerId.isEmpty())
            {
                m_customerTableModel->replaceComputerId(
                            selIndexes.first(),
                            selComputerIds.first().row(),
                            newComputerId);
            }
        }
        else
        {
            QMessageBox::information(
                        this,
                        tr("No computer ID"),
                        tr("You need to select a computer ID"));
        }
    }
    else
    {
        QMessageBox::information(
                    this,
                    tr("No customer"),
                    tr("You need to select a customer"));
    }
}

void PaneSettingsCustomers::removeComputerId()
{
    auto selComputerIds = ui->listViewIps->selectionModel()->selectedIndexes();
    if (selComputerIds.size() > 0)
    {
        const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
        if (selIndexes.size() > 0)
        {
            m_customerTableModel->removeComputerId(
                        selIndexes.first(),
                        selComputerIds.first().row());
        }
    }
    else
    {
        QMessageBox::information(
                    this,
                    tr("No computer ID"),
                    tr("You need to select a computer ID"));
    }
}

void PaneSettingsCustomers::selectCustomer(
        const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0)
    {
        auto &computerIdModel = m_customerTableModel->getComputerIds(
                    selected.indexes().first());
        ui->listViewIps->setModel(&computerIdModel);
        static QSet<QStringListModel *> connected;
        if (!connected.contains(&computerIdModel))
        {
            connect(&computerIdModel,
                    &QStringListModel::dataChanged,
                    this, [](){

            });
            connected.insert(&computerIdModel);
        }
    }
}

