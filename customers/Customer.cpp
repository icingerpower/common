#include "Customer.h"

QString Customer::KEY_ID{"id"};
QString Customer::KEY_NAME{"name"};
QString Customer::KEY_PHONE{"phone"};
QString Customer::KEY_EMAIL{"email"};
QString Customer::KEY_EMAIL_DATA{"emailData"};
QString Customer::KEY_DATE_PAYMENT{"paymentDate"};
QString Customer::KEY_DATE_LAST_PAYMENT{"lastPaymentDate"};
QString Customer::KEY_MAX_IPS{"maxEthernetAddresses"};
QString Customer::KEY_COMPUTER_ID_ADDRESSES{"ethernetAddress"};
QString Customer::KEY_FOREVER_ACCESS{"foreverAccess"};

Customer::Customer(
    const QString &id,
    const QString &email,
    const QString &name,
    const QDate &datePayment,
    int maxComputerIds)
{
    m_id = id;
    m_email = email;
    m_emailData = email;
    m_name = name;
    m_datePayment = datePayment;
    m_dateLastPayment = datePayment;
    m_maxComputerIds = maxComputerIds;
    m_foreverAccess = false;
}

Customer::Customer(const QHash<QString, QVariant> &hash)
{
    m_id = hash[KEY_ID].toString();
    m_name = hash[KEY_NAME].toString();
    m_phoneNumber = hash[KEY_PHONE].toString();
    m_email = hash[KEY_EMAIL].toString();
    m_emailData = hash.value(KEY_EMAIL_DATA, m_email).toString();
    m_datePayment = hash[KEY_DATE_PAYMENT].toDate();
    m_dateLastPayment = hash[KEY_DATE_LAST_PAYMENT].toDate();
    m_maxComputerIds = hash[KEY_MAX_IPS].toInt();
    m_computerIds.setStringList(hash[KEY_COMPUTER_ID_ADDRESSES].toStringList());
    m_foreverAccess = hash.value(KEY_COMPUTER_ID_ADDRESSES, false).toBool();
}

QHash<QString, QVariant> Customer::toHash() const
{
    QHash<QString, QVariant> hash;
    hash[KEY_ID] = m_id;
    hash[KEY_NAME] = m_name;
    hash[KEY_PHONE] = m_phoneNumber;
    hash[KEY_EMAIL] = m_email;
    hash[KEY_EMAIL_DATA] = m_emailData;
    hash[KEY_DATE_PAYMENT] = m_datePayment;
    hash[KEY_DATE_LAST_PAYMENT] = m_dateLastPayment;
    hash[KEY_MAX_IPS] = m_maxComputerIds;
    hash[KEY_COMPUTER_ID_ADDRESSES] = m_computerIds.stringList();
    hash[KEY_FOREVER_ACCESS] = m_foreverAccess;
    return hash;
}

void Customer::addComputerId(const QString &computerId)
{
    if (m_maxComputerIds > m_computerIds.rowCount())
    {
        m_computerIds.insertRows(0, 1);
        m_computerIds.setData(m_computerIds.index(0, 0), computerId);
    }
}

bool Customer::foreverAccess() const
{
    return m_foreverAccess;
}

void Customer::setForeverAccess(bool newForeverAccess)
{
    m_foreverAccess = newForeverAccess;
}

const QString &Customer::emailData() const
{
    return m_emailData;
}

void Customer::setEmailData(const QString &newEmailData)
{
    m_emailData = newEmailData;
}

const QString &Customer::id() const
{
    return m_id;
}

const QString &Customer::phoneNumber() const
{
    return m_phoneNumber;
}

void Customer::setPhoneNumber(const QString &newPhoneNumber)
{
    m_phoneNumber = newPhoneNumber;
}

const QString &Customer::name() const
{
    return m_name;
}

void Customer::setName(const QString &newName)
{
    m_name = newName;
}

const QString &Customer::email() const
{
    return m_email;
}

void Customer::setEmail(const QString &newEmail)
{
    m_email = newEmail;
}

QDate Customer::datePayment() const
{
    return m_datePayment;
}

void Customer::setDatePayment(const QDate &newDatePayment)
{
    m_datePayment = newDatePayment;
}

QDate Customer::dateLastPayment() const
{
    return m_dateLastPayment;
}

void Customer::setDateLastPayment(const QDate &newDateLastPayment)
{
    m_dateLastPayment = newDateLastPayment;
}

int Customer::maxComputerIds() const
{
    return m_maxComputerIds;
}

void Customer::setMaxComputerIds(int newMaxComputerIds)
{
    m_maxComputerIds = newMaxComputerIds;
}

QStringListModel &Customer::computerUniqueIds()
{
    return m_computerIds;
}

void Customer::setComputerIds(const QStringList &newMaxComputerIds)
{
    m_computerIds.setStringList(newMaxComputerIds);
}

void Customer::replaceComputerId(int rowIndex, const QString &afterEth)
{
    m_computerIds.setData(
                m_computerIds.index(rowIndex, 0),
                afterEth);
}

void Customer::removeComputerId(int rowIndex)
{
    m_computerIds.removeRows(rowIndex, 1);
}
