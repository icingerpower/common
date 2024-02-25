#ifndef CURRENCYMANAGER_H
#define CURRENCYMANAGER_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

class CurrencyManager
{
public:
    static CurrencyManager *instance();
    static const QStringList *currencies();
    static const QString currency(const QString &countryCode);
    //static const QStringList *currenciesUE();

private:
    CurrencyManager();
};

#endif // CURRENCYMANAGER_H
