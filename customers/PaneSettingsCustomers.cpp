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
    connect(ui->buttonAddEthAddress,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::addEthernetAddress);
    connect(ui->buttonRemoveEthAddress,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::removeEthernetAddress);
    connect(ui->buttonReplaceEthernetAddress,
            &QPushButton::clicked,
            this,
            &PaneSettingsCustomers::replaceEthernetAddress);
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

void PaneSettingsCustomers::addEthernetAddress()
{
    const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        const QString &ethernet = QInputDialog::getText(
                    this, tr("Ethernet address"), tr("Enter the ethernet address"));
        if (!ethernet.isEmpty())
        {
            m_customerTableModel->addEthernetAddress(
                        selIndexes.first(),
                        ethernet);
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

void PaneSettingsCustomers::replaceEthernetAddress()
{
    const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
    if (selIndexes.size() > 0)
    {
        auto selEthernets = ui->listViewIps->selectionModel()->selectedIndexes();
        if (selEthernets.size() > 0)
        {
            const QString &newEthernet = QInputDialog::getText(
                        this, tr("Ethernet address"), tr("Enter the ethernet address"));
            if (!newEthernet.isEmpty())
            {
                m_customerTableModel->replaceEthernetAddress(
                            selIndexes.first(),
                            selEthernets.first().row(),
                            newEthernet);
            }
        }
        else
        {
            QMessageBox::information(
                        this,
                        tr("No ethernet address"),
                        tr("You need to select an ethernet address"));
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

void PaneSettingsCustomers::removeEthernetAddress()
{
    auto selEthernets = ui->listViewIps->selectionModel()->selectedIndexes();
    if (selEthernets.size() > 0)
    {
        const auto &selIndexes = ui->tableViewCustomers->selectionModel()->selectedIndexes();
        if (selIndexes.size() > 0)
        {
            m_customerTableModel->removeEthernetAddress(
                        selIndexes.first(),
                        selEthernets.first().row());
        }
    }
    else
    {
        QMessageBox::information(
                    this,
                    tr("No ethernet address"),
                    tr("You need to select an ethernet address"));
    }
}

void PaneSettingsCustomers::selectCustomer(
        const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.size() > 0)
    {
        auto &ethernetModel = m_customerTableModel->getEthernetAddresses(
                    selected.indexes().first());
        ui->listViewIps->setModel(&ethernetModel);
        static QSet<QStringListModel *> connected;
        if (!connected.contains(&ethernetModel))
        {
            connect(&ethernetModel,
                    &QStringListModel::dataChanged,
                    this, [](){

            });
            connected.insert(&ethernetModel);
        }
    }
}

