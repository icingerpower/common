#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QString>
#include <QStringListModel>
#include <QDate>

class Customer
{
public:
    Customer(const QString &id,
             const QString &email,
             const QString &name,
             const QDate &datePayment,
             int maxEthernetAddresses);
    Customer(const QHash<QString, QVariant> &hashedData);
    //Customer(Customer&& other) noexcept;

    QHash<QString, QVariant> toHash() const;

    const QString &id() const;

    const QString &phoneNumber() const;
    void setPhoneNumber(const QString &newPhoneNumber);

    const QString &name() const;
    void setName(const QString &newName);

    const QString &email() const;
    void setEmail(const QString &newEmail);

    QDate datePayment() const;
    void setDatePayment(const QDate &newDatePayment);

    QDate dateLastPayment() const;
    void setDateLastPayment(const QDate &newDateLastPayment);

    int maxEthernetAddresses() const;
    void setMaxEthernetAddresses(int newMaxEthernetAddresses);

    QStringListModel &ethernetAddresses();
    void setEthernetAddresses(const QStringList &newEthernetAddresses);
    void replaceEthernetAddress(int rowIndex, const QString &afterEth);
    void removeEthernetAddress(int rowIndex);
    void addEthernetAddress(const QString &ethernetAddress);

    bool foreverAccess() const;
    void setForeverAccess(bool newForeverAccess);

    const QString &emailData() const;
    void setEmailData(const QString &newEmailData);

protected:
    QString m_id;
    QString m_name;
    QString m_phoneNumber;
    QString m_email;
    QString m_emailData;
    QDate m_datePayment;
    QDate m_dateLastPayment;
    int m_maxEthernetAddresses;
    QStringListModel m_ethernetAddresses;
    bool m_foreverAccess;
    static QString KEY_ID;
    static QString KEY_NAME;
    static QString KEY_PHONE;
    static QString KEY_EMAIL;
    static QString KEY_EMAIL_DATA;
    static QString KEY_DATE_PAYMENT;
    static QString KEY_DATE_LAST_PAYMENT;
    static QString KEY_MAX_IPS;
    static QString KEY_ETHERNET_ADDRESSES;
    static QString KEY_FOREVER_ACCESS;
};

#endif // CUSTOMER_H
