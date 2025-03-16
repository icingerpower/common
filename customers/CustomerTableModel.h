#ifndef CUSTOMERTABLEMODEL_H
#define CUSTOMERTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringListModel>
#include <QSharedPointer>

#include "Customer.h"

#define REGISTER_CUSTOMER_META_TYPES \
    qRegisterMetaType<QList<QHash<QString, QVariant>>>(); \
    qRegisterMetaType<QHash<QString, QDate>>();

class CustomerTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static QString KEY_SETTINGS_CUSTOMER_COMPUTER_IDS;
    explicit CustomerTableModel(const QString &settingsFilePath,
                                const QString &settingsFilePathSecret,
                                QObject *parent = nullptr);
    void addCustomer(QSharedPointer<Customer> customer);
    void removeCustomer(const QModelIndex &index);
    QStringListModel &getComputerIds(const QModelIndex &index);
    void removeComputerId(const QModelIndex &indexCustomer, int rowIndexId);
    void addComputerId(const QModelIndex &indexCustomer, const QString &computerId);
    void replaceComputerId(const QModelIndex &indexCustomer,
                                int rowIndexId,
                                const QString &computerIdAfter);
    bool isComputerIdAllowed() const;
    bool wasComputerIdAllowed() const;
    static QString getUniqueMachineIdentifier();

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QHash<QString, QDate> readMacAddresses() const;

private:
    struct ColInfo{
        bool editable;
        QString colId;
        QString colName;
        std::function<QVariant(const Customer &customer)> funcValue;
        std::function<void(Customer &customer, const QVariant &)> funcSetVue;
    };
    QList<ColInfo> m_colInfos;
    QList<QSharedPointer<Customer>> m_customers;
    void saveInSettings();
    void loadFromSettings();
    static QString KEY_SETTINGS_CUSTOMERS;
    void _connectCustomer(Customer *customer);
    QString m_settingsFilePath;
    QString m_settingsFilePathSecret;
    static QString getMacOSHardwareUUID();
    static QString getLinuxMachineId();
    static QString getMachineUUIDWindows();
};

#endif // CUSTOMERTABLEMODEL_H
