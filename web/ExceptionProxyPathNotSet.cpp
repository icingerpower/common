#include <QObject>

#include "ExceptionProxyPathNotSet.h"


//----------------------------------------------------------
void ExceptionProxyPathNotSet::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionProxyPathNotSet *ExceptionProxyPathNotSet::clone() const
{
    return new ExceptionProxyPathNotSet(*this);
}
//----------------------------------------------------------
QString ExceptionProxyPathNotSet::error() const
{
    if (m_error.isEmpty()) {
        return QObject::tr("The file with proxy information is not set.");
    }
    return m_error;
}
//----------------------------------------------------------
void ExceptionProxyPathNotSet::setError(const QString &newError)
{
    m_error = newError;
}
//----------------------------------------------------------
