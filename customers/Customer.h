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
             int maxComputerIds);
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

    int maxComputerIds() const;
    void setMaxComputerIds(int newMaxComputerIds);

    QStringListModel &computerUniqueIds();
    void setComputerIds(const QStringList &newComputerIds);
    void replaceComputerId(int rowIndex, const QString &afterEth);
    void removeComputerId(int rowIndex);
    void addComputerId(const QString &computerId);

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
    int m_maxComputerIds;
    QStringListModel m_computerIds;
    bool m_foreverAccess;
    static QString KEY_ID;
    static QString KEY_NAME;
    static QString KEY_PHONE;
    static QString KEY_EMAIL;
    static QString KEY_EMAIL_DATA;
    static QString KEY_DATE_PAYMENT;
    static QString KEY_DATE_LAST_PAYMENT;
    static QString KEY_MAX_IPS;
    static QString KEY_COMPUTER_ID_ADDRESSES;
    static QString KEY_FOREVER_ACCESS;
};

#endif // CUSTOMER_H
