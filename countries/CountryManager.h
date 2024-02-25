#ifndef COUNTRYMANAGER_H
#define COUNTRYMANAGER_H

#include <QtCore/qstring.h>

class CountryManager
{
public:
    static QString FRANCE;
    static QString GERMANY;
    static QString SPAIN;
    static QString ITALY;
    static QString SWEDEN;
    static QString POLAND;
    static QString CZECH;
    static QString NETHERLAND;
    static QString BELGIUM;
    static QString AUSTRIA;
    static QString BULGARIA;
    static QString CHYPRE;
    static QString CROATIA;
    static QString DENMARK;
    static QString ESTONIA;
    static QString FINLAND;
    static QString GREECE;
    static QString HONGRY;
    static QString IRELAND;
    static QString IRELAND_NORTHEN;
    static QString LATVIA;
    static QString LUXEMBOURG;
    static QString MALTA;
    static QString PORTUGAL;
    static QString ROMANIA;
    static QString SLOVAKIA;
    static QString SLOVENIA;
    static QString LITHUANIA;
    static QString UK;
    static QString EU;
    static QString CHINA;
    static CountryManager *instance();
    static const QStringList *countryCodes();
    static const QStringList *countryNames();
    static const QStringList *countryNamesSorted();
    static const QStringList *countriesFbaCentersUE();
    static const QStringList *countriesCodeUE();
    static const QStringList *countriesCodeUE(int year);
    static const QStringList *countriesNamesUE(int year);
    static const QStringList *countriesCodeUEfrom2020();
    static const QStringList *countriesNamesUEfrom2020();
    static const QStringList *months();
    struct CountryInfo{
        QString currency;
        QString currencySymbol;
        bool symbolAtEnd;
    };
    static const QHash<QString, CountryInfo> *countryInfos();
    const QString &getCurrencySymbol(const QString &countryCode) const;
    const QString &getCurrency(const QString &countryCode) const;
    const bool &getCurrencyAtEnd(const QString &countryCode) const;

    QString countryCodeDomTom(const QString &countryCode, const QString &postalCode) const;
    QString countryName(const QString &countryCode) const;
    QString countryCode(const QString &countryName) const;
    QString countryCurrency(const QString &countryCode) const;
    QString countryCurrencySymbol(const QString &countryCode) const;

private:
    CountryManager();
};

#endif // COUNTRYMANAGER_H
