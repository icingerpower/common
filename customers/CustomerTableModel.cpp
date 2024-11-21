#include <QSettings>
#include <QNetworkInterface>

#include "CustomerTableModel.h"

QString CustomerTableModel::KEY_SETTINGS_CUSTOMERS{"hashkeywords"};
QString CustomerTableModel::KEY_SETTINGS_CUSTOMER_MAC_ADDRESSES{"hashtranslation"};


CustomerTableModel::CustomerTableModel(
    const QString &settingsFilePath,
    const QString &settingsFilePathSecret,
    QObject *parent)
    : QAbstractTableModel(parent)
{
    m_settingsFilePath = settingsFilePath;
    m_settingsFilePathSecret = settingsFilePathSecret;
    loadFromSettings();
    m_colInfos << ColInfo{true,
                  "id-customer-name",
                  tr("Name", "The customer name"),
            [](const Customer &customer) -> QVariant {
                              return customer.name();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setName(value.toString());
                   }};
    m_colInfos << ColInfo{true,
                  "id-customer-email",
                  tr("Email"),
            [](const Customer &customer) -> QVariant {
                              return customer.email();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setEmail(value.toString());
                   }};
    m_colInfos << ColInfo{true,
                  "id-number-ips",
                  tr("Number of ips"),
            [](const Customer &customer) -> QVariant {
                              return customer.maxEthernetAddresses();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setMaxEthernetAddresses(value.toInt());
                   }};
    m_colInfos << ColInfo{true,
                  "id-payment-date-last",
                  tr("Date last payment"),
            [](const Customer &customer) -> QVariant {
                              return customer.dateLastPayment();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setDateLastPayment(value.toDate());
                   }};
    m_colInfos << ColInfo{true,
                  "id-forever-access",
                  tr("Forever access"),
            [](const Customer &customer) -> QVariant {
                              return customer.foreverAccess();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setForeverAccess(value.toBool());
                   }};
    m_colInfos << ColInfo{true,
                  "id-customer-email-data",
                  tr("Email data share", "Email to which data should be shared"),
            [](const Customer &customer) -> QVariant {
                              return customer.emailData();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setEmailData(value.toString());
                   }};
    m_colInfos << ColInfo{true,
                  "id-payment-date-first",
                  tr("Date 1st payment"),
            [](const Customer &customer) -> QVariant {
                              return customer.datePayment();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setDatePayment(value.toDate());
                   }};
    m_colInfos << ColInfo{true,
                  "id-customer-phone",
                  tr("Phone number"),
            [](const Customer &customer) -> QVariant {
                              return customer.phoneNumber();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setPhoneNumber(value.toString());
                   }};
    m_colInfos << ColInfo{false,
                  "id-customer",
                  tr("ID"),
            [](const Customer &customer) -> QVariant {
                              return customer.id();
                   },
            [](Customer &, const QVariant &) {
                   }};
}

void CustomerTableModel::addCustomer(QSharedPointer<Customer> customer)
{
    beginInsertRows(QModelIndex{}, 0, 0);
    m_customers.insert(0, customer);
    _connectCustomer(customer.data());
    saveInSettings();
    endInsertRows();
}

void CustomerTableModel::removeCustomer(const QModelIndex &index)
{
    beginRemoveRows(QModelIndex{}, index.row(), index.row());
    m_customers.remove(index.row());
    saveInSettings();
    endRemoveRows();
}

QStringListModel &CustomerTableModel::getEthernetAddresses(const QModelIndex &index)
{
    return m_customers[index.row()]->ethernetAddresses();
}

void CustomerTableModel::removeEthernetAddress(
        const QModelIndex &indexCustomer, int rowIndexEth)
{
    m_customers[indexCustomer.row()]->removeEthernetAddress(rowIndexEth);
    saveInSettings();
}

void CustomerTableModel::addEthernetAddress(
        const QModelIndex &indexCustomer, const QString &ethernetAddress)
{
    m_customers[indexCustomer.row()]->addEthernetAddress(ethernetAddress);
    saveInSettings();
}

void CustomerTableModel::replaceEthernetAddress(
        const QModelIndex &indexCustomer,
        int rowIndexEth,
        const QString &ethernetAddressAfter)
{
    m_customers[indexCustomer.row()]->replaceEthernetAddress(
                rowIndexEth, ethernetAddressAfter);
    saveInSettings();
}

QVariant CustomerTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        return m_colInfos[section].colName;
    }
    return QVariant{};
}

int CustomerTableModel::rowCount(const QModelIndex &) const
{
    return m_customers.size();
}

int CustomerTableModel::columnCount(const QModelIndex &) const
{
    return m_colInfos.size();
}

QVariant CustomerTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_colInfos[index.column()].funcValue(*m_customers[index.row()]);
    }
    else if (role == Qt::CheckStateRole)
    {
        static auto boolTypeId = QVariant{true}.typeId();
        auto value = m_colInfos[index.column()].funcValue(*m_customers[index.row()]);
        if (value.typeId() == boolTypeId)
        {
            qDebug() << "ok4";
            return value.toBool() ? Qt::Checked : Qt::Unchecked;
        }
    }
    return QVariant{};
}

bool CustomerTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
     if (index.isValid() && value != data(index, role))
     {
         m_colInfos[index.column()].funcSetVue(*m_customers[index.row()], value);
         saveInSettings();
         emit dataChanged(index, index, QList<int>{role});
         return true;
     }
     return false;
}

void CustomerTableModel::sort(int column, Qt::SortOrder order)
{
}

Qt::ItemFlags CustomerTableModel::flags(const QModelIndex &index) const
{
    if (m_colInfos[index.column()].editable)
    {
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QHash<QString, QDate> CustomerTableModel::readMacAddresses() const
{
    QSettings settings{m_settingsFilePath, QSettings::IniFormat};
    return settings.value(
                KEY_SETTINGS_CUSTOMER_MAC_ADDRESSES,
                QVariant::fromValue(QHash<QString, QDate>{})).value<QHash<QString, QDate>>();
}

QString CustomerTableModel::ethernetAddress()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces)
    {
        if  (!interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            bool valid = interface.isValid();
            if (valid)
            {
                auto hardwardAddress = interface.hardwareAddress();
                if (!hardwardAddress.contains("00:00:00"))
                {
                    return hardwardAddress;
                }
            }
        }
        //}
    }
    return QString{};
}


void CustomerTableModel::saveInSettings()
{
    QSettings settingsLocal{m_settingsFilePathSecret, QSettings::IniFormat};
    QList<QHash<QString, QVariant>> customers;
    for (const auto &customer : m_customers)
    {
        customers << customer->toHash();
    }
    settingsLocal.setValue(KEY_SETTINGS_CUSTOMERS, QVariant::fromValue(customers));

    QSettings settings{m_settingsFilePath, QSettings::IniFormat};
    QHash<QString, QDate> allEthernetAddresses;
    for (const auto &customer : m_customers)
    {
        const auto &addresses = customer->ethernetAddresses().stringList();
        QDate endDate = customer->dateLastPayment().addYears(1);
        if (customer->foreverAccess())
        {
            endDate = QDate::currentDate().addYears(99);
        }
        if (endDate > QDate::currentDate())
        {
            for (const auto &ethAddress : addresses)
            {
                allEthernetAddresses.insert(ethAddress, endDate);
            }
        }
    }
    settings.setValue(KEY_SETTINGS_CUSTOMER_MAC_ADDRESSES,
                       QVariant::fromValue(allEthernetAddresses));
}

void CustomerTableModel::loadFromSettings()
{
    QSettings settingsLocal{m_settingsFilePathSecret, QSettings::IniFormat};
    const QList<QHash<QString, QVariant>> &customers
        = settingsLocal.value(KEY_SETTINGS_CUSTOMERS)
              .value<QList<QHash<QString, QVariant>>>();
    for (const auto & hash : customers)
    {
        m_customers.append(QSharedPointer<Customer>{new Customer{hash}});
        _connectCustomer(m_customers.last().data());
    }
}

void CustomerTableModel::_connectCustomer(Customer *customer)
{
    connect(&customer->ethernetAddresses(),
            &QStringListModel::dataChanged,
            this, [this](){
        saveInSettings();
    });
}
bool CustomerTableModel::isEthernetAddressAllowed() const
{
    const auto &allowedAddresses = readMacAddresses();
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces)
    {
        if  (!interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            auto it = allowedAddresses.find(interface.hardwareAddress());
            if (it != allowedAddresses.end() && it.value() >= QDate::currentDate())
            {
                return true;
            }
        }
    }
    return false;
}

bool CustomerTableModel::wasEthernetAddressAllowed() const
{
    const auto &allowedAddresses = readMacAddresses();
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces)
    {
        if  (!interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            auto it = allowedAddresses.find(interface.hardwareAddress());
            if (it != allowedAddresses.end())
            {
                return true;
            }
        }
    }
    return false;
}

