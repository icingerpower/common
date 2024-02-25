#include "ExceptionCurrencyRate.h"


//----------------------------------------------------------
void ExceptionCurrencyRate::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionCurrencyRate *ExceptionCurrencyRate::clone() const
{
    return new ExceptionCurrencyRate(*this);
}
//----------------------------------------------------------
QString ExceptionCurrencyRate::url() const
{
    return m_urlError;
}
//----------------------------------------------------------
void ExceptionCurrencyRate::setUrl(const QString &urlError)
{
    m_urlError = urlError;
}
//----------------------------------------------------------
