#include "Customer.h"

QString Customer::KEY_ID{"id"};
QString Customer::KEY_NAME{"name"};
QString Customer::KEY_PHONE{"phone"};
QString Customer::KEY_EMAIL{"email"};
QString Customer::KEY_EMAIL_DATA{"emailData"};
QString Customer::KEY_DATE_PAYMENT{"paymentDate"};
QString Customer::KEY_DATE_LAST_PAYMENT{"lastPaymentDate"};
QString Customer::KEY_MAX_IPS{"maxEthernetAddresses"};
QString Customer::KEY_ETHERNET_ADDRESSES{"ethernetAddress"};
QString Customer::KEY_FOREVER_ACCESS{"foreverAccess"};

Customer::Customer(
    const QString &id,
    const QString &email,
    const QString &name,
    const QDate &datePayment,
    int maxEthernetAddresses)
{
    m_id = id;
    m_email = email;
    m_emailData = email;
    m_name = name;
    m_datePayment = datePayment;
    m_dateLastPayment = datePayment;
    m_maxEthernetAddresses = maxEthernetAddresses;
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
    m_maxEthernetAddresses = hash[KEY_MAX_IPS].toInt();
    m_ethernetAddresses.setStringList(hash[KEY_ETHERNET_ADDRESSES].toStringList());
    m_foreverAccess = hash.value(KEY_ETHERNET_ADDRESSES, false).toBool();
}

/*
Customer::Customer(Customer&& other) noexcept
    : m_id(std::move(other.m_id)),
      m_name(std::move(other.m_name)),
      m_phoneNumber(std::move(other.m_phoneNumber)),
      m_email(std::move(other.m_email)),
      m_datePayment(std::move(other.m_datePayment)),
      m_dateLastPayment(std::move(other.m_dateLastPayment)),
      m_maxEthernetAddresses(other.m_maxEthernetAddresses)
      m_foreverAccess(other.m_foreverAccess)
{
    m_ethernetAddresses.setStringList(other.m_ethernetAddresses.stringList());
    other.m_maxEthernetAddresses = 0;
}
//*/

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
    hash[KEY_MAX_IPS] = m_maxEthernetAddresses;
    hash[KEY_ETHERNET_ADDRESSES] = m_ethernetAddresses.stringList();
    hash[KEY_FOREVER_ACCESS] = m_foreverAccess;
    return hash;
}

void Customer::addEthernetAddress(const QString &ethernet)
{
    if (m_maxEthernetAddresses > m_ethernetAddresses.rowCount())
    {
        m_ethernetAddresses.insertRows(0, 1);
        m_ethernetAddresses.setData(m_ethernetAddresses.index(0, 0), ethernet);
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

int Customer::maxEthernetAddresses() const
{
    return m_maxEthernetAddresses;
}

void Customer::setMaxEthernetAddresses(int newMaxEthernetAddresses)
{
    m_maxEthernetAddresses = newMaxEthernetAddresses;
}

QStringListModel &Customer::ethernetAddresses()
{
    return m_ethernetAddresses;
}

void Customer::setEthernetAddresses(const QStringList &newEthernetAddresses)
{
    m_ethernetAddresses.setStringList(newEthernetAddresses);
}

void Customer::replaceEthernetAddress(int rowIndex, const QString &afterEth)
{
    m_ethernetAddresses.setData(
                m_ethernetAddresses.index(rowIndex, 0),
                afterEth);
}

void Customer::removeEthernetAddress(int rowIndex)
{
    m_ethernetAddresses.removeRows(rowIndex, 1);
}
