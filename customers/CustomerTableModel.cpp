#include <QSettings>
#include <QProcess>
#include <QNetworkInterface>
#include <QFile>
#include <QRegularExpression>

#include "CustomerTableModel.h"

QString CustomerTableModel::KEY_SETTINGS_CUSTOMERS{"hashkeywords"};
QString CustomerTableModel::KEY_SETTINGS_CUSTOMER_COMPUTER_IDS{"hashtranslation"};


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
                              return customer.maxComputerIds();
                   },
            [](Customer &customer, const QVariant &value) {
        customer.setMaxComputerIds(value.toInt());
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

QStringListModel &CustomerTableModel::getComputerIds(const QModelIndex &index)
{
    return m_customers[index.row()]->computerUniqueIds();
}

void CustomerTableModel::removeComputerId(
        const QModelIndex &indexCustomer, int rowIndexEth)
{
    m_customers[indexCustomer.row()]->removeComputerId(rowIndexEth);
    saveInSettings();
}

void CustomerTableModel::addComputerId(
        const QModelIndex &indexCustomer, const QString &computerId)
{
    m_customers[indexCustomer.row()]->addComputerId(computerId);
    saveInSettings();
}

void CustomerTableModel::replaceComputerId(
        const QModelIndex &indexCustomer,
        int rowIndexId,
        const QString &computerIdAfter)
{
    m_customers[indexCustomer.row()]->replaceComputerId(
                rowIndexId, computerIdAfter);
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
    bool continas = settings.contains(KEY_SETTINGS_CUSTOMER_COMPUTER_IDS);
    return settings.value(
                KEY_SETTINGS_CUSTOMER_COMPUTER_IDS,
                QVariant::fromValue(QHash<QString, QDate>{})).value<QHash<QString, QDate>>();
}

QString CustomerTableModel::getUniqueMachineIdentifier()
{
    QString id;

#ifdef Q_OS_WIN
    // 1) Registry MachineGuid
    id = getWindowsMachineGuid();
    // 2) WMIC UUID
    if (id.isEmpty()) id = getWindowsWmicUuid();

#elif defined(Q_OS_MAC)
    // 1) IOKit IOPlatformUUID
    id = getMacIOPlatformUUID();
    // 2) system_profiler Serial
    if (id.isEmpty()) id = getMacSystemProfilerSerial();

#elif defined(Q_OS_LINUX)
    // 1) /etc/machine-id
    id = getLinuxMachineId("/etc/machine-id");
    // 2) /var/lib/dbus/machine-id
    if (id.isEmpty()) id = getLinuxMachineId("/var/lib/dbus/machine-id");

#else
    id.clear();
#endif

    // 3) Last resort: first non‐empty hardware MAC
    if (id.isEmpty()) id = getFirstHardwareMac();

    return id;
}

// ——————————————————————————————————————————————
// Windows methods
// ——————————————————————————————————————————————
#ifdef Q_OS_WIN
#include <windows.h>
#include <winreg.h>
QString CustomerTableModel::getWindowsMachineGuid()
{
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Cryptography",
                      0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
        return {};

    WCHAR buf[256];
    DWORD bufSize = sizeof(buf);
    if (RegQueryValueExW(hKey, L"MachineGuid", nullptr, nullptr,
                         (LPBYTE)buf, &bufSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return {};
    }
    RegCloseKey(hKey);
    return QString::fromWCharArray(buf);
}

QString CustomerTableModel::getWindowsWmicUuid()
{
    QProcess p;
    p.start("wmic", QStringList() << "csproduct" << "get" << "uuid");
    p.waitForFinished();
    const auto out = QString::fromLocal8Bit(p.readAllStandardOutput())
                        .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    return (out.size() >= 2) ? out.at(1).trimmed() : QString();
}
#endif

// ——————————————————————————————————————————————
// macOS methods
// ——————————————————————————————————————————————
#ifdef Q_OS_MAC
QString CustomerTableModel::getMacIOPlatformUUID()
{
    QProcess p;
    p.start("ioreg", QStringList() << "-rd1" << "-c" << "IOPlatformExpertDevice");
    p.waitForFinished();
    const auto output = p.readAllStandardOutput();
    QRegularExpression re(R"("IOPlatformUUID"\s*=\s*"(.+)")");
    auto m = re.match(output);
    return m.hasMatch() ? m.captured(1) : QString();
}

QString CustomerTableModel::getMacSystemProfilerSerial()
{
    QProcess p;
    p.start("system_profiler", QStringList() << "SPHardwareDataType");
    p.waitForFinished();
    const auto out = QString::fromLocal8Bit(p.readAllStandardOutput())
                          .split('\n');
    for (auto &line : out) {
        if (line.contains("Serial Number", Qt::CaseInsensitive)) {
            auto parts = line.split(':');
            if (parts.size() == 2) return parts[1].trimmed();
        }
    }
    return {};
}
#endif

// ——————————————————————————————————————————————
// Linux methods
// ——————————————————————————————————————————————
#ifdef Q_OS_LINUX
QString CustomerTableModel::getLinuxMachineId(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) return {};
    const auto id = QString::fromUtf8(f.readLine()).trimmed();
    f.close();
    return id;
}
#endif

// ——————————————————————————————————————————————
// Fallback: first non‐empty MAC
// ——————————————————————————————————————————————
QString CustomerTableModel::getFirstHardwareMac()
{
    static const QStringList virtualPrefixes = {
        "veth",   // Docker, Linux bridges
        "docker",
        "virbr",  // libvirt
        "vmnet",  // VMware
        "vboxnet" // VirtualBox
    };

    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        // must be up, non-loopback, have a hardware address
        if (!iface.flags().testFlag(QNetworkInterface::IsUp) ||
            iface.flags().testFlag(QNetworkInterface::IsLoopBack) ||
            iface.hardwareAddress().isEmpty()) {
            continue;
        }

        // optional: skip known virtual NIC name prefixes
        bool isVirtualName = false;
        for (const QString &pfx : virtualPrefixes) {
            if (iface.name().startsWith(pfx, Qt::CaseInsensitive)) {
                isVirtualName = true;
                break;
            }
        }
        if (isVirtualName)
            continue;

        // return the MAC without separators
        return iface.hardwareAddress().replace(":", "");
    }

    return QString();
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
    QHash<QString, QDate> allComputerIds;
    for (const auto &customer : m_customers)
    {
        const auto &addresses = customer->computerUniqueIds().stringList();
        QDate endDate = customer->dateLastPayment().addYears(1);
        if (customer->foreverAccess())
        {
            endDate = QDate::currentDate().addYears(99);
        }
        if (endDate > QDate::currentDate())
        {
            for (const auto &ethAddress : addresses)
            {
                allComputerIds.insert(ethAddress, endDate);
            }
        }
    }
    settings.setValue(KEY_SETTINGS_CUSTOMER_COMPUTER_IDS,
                       QVariant::fromValue(allComputerIds));
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
    connect(&customer->computerUniqueIds(),
            &QStringListModel::dataChanged,
            this, [this](){
        saveInSettings();
    });
}
bool CustomerTableModel::isComputerIdAllowed() const
{
    const auto &allowedAddresses = readMacAddresses();
    const auto &uniqueManchineId = getUniqueMachineIdentifier();
    auto it = allowedAddresses.find(uniqueManchineId);
    if (it != allowedAddresses.end() && it.value() >= QDate::currentDate())
    {
        return true;
    }
    /*
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
    //*/
    return false;
}

bool CustomerTableModel::wasComputerIdAllowed() const
{
    const auto &allowedAddresses = readMacAddresses();
    const auto &uniqueManchineId = getUniqueMachineIdentifier();
    auto it = allowedAddresses.find(uniqueManchineId);
    return  it != allowedAddresses.end();
    /*
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
    //*/
}

