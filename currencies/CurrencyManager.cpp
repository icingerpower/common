#include <QtCore/qhash.h>

#include "CurrencyManager.h"

//----------------------------------------------------------
CurrencyManager *CurrencyManager::instance()
{
    static CurrencyManager instance;
    return &instance;
}
//----------------------------------------------------------
const QStringList *CurrencyManager::currencies()
{
    static QStringList currencies
            = []() -> QStringList {
            QStringList allCurrencies
            = {"EUR", "USD", "CNY", "GBP", "AUD"
            , "CHF", "CAD", "SGD", "JPY"};
            std::sort(allCurrencies.begin(), allCurrencies.end());
            return allCurrencies;
}();
    return &currencies;
}
//----------------------------------------------------------
const QString CurrencyManager::currency(const QString &countryCode)
{
    static QHash<QString, QString> mapping
            = {{"FR", "EUR"}, {"DE", "EUR"}, {"PL", "PLN"}
               , {"SE", "SEK"}, {"CZ", "CZK"}, {"DE", "EUR"}
               , {"CN", "CNY"}, {"US", "USD"}, {"CA", "CAD"}
               , {"CH", "CHF"}, {"AU", "AUD"}, {"JP", "JPY"}
              };
    return mapping[countryCode];
}
//----------------------------------------------------------
CurrencyManager::CurrencyManager()
{
}
//----------------------------------------------------------
