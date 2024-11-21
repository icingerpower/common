#ifndef PANESETTINGSCUSTOMERS_H
#define PANESETTINGSCUSTOMERS_H

#include <QWidget>
#include <QItemSelection>

namespace Ui {
class PaneSettingsCustomers;
}

class CustomerTableModel;

class PaneSettingsCustomers : public QWidget
{
    Q_OBJECT

public:
    explicit PaneSettingsCustomers(QWidget *parent = nullptr);
    ~PaneSettingsCustomers();
    void init(
        const QString &settingsFilePath,
        const QString &settingsFilePathSecret);

public slots:
    void addCustomer();
    void removeCustomer();
    void search();
    void searchReset();
    void addEthernetAddress();
    void replaceEthernetAddress();
    void removeEthernetAddress();

protected slots:
    void selectCustomer(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Ui::PaneSettingsCustomers *ui;
    void _connectSlots();
    CustomerTableModel *m_customerTableModel;
};

#endif // PANESETTINGSCUSTOMERS_H
