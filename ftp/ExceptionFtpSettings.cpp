#include "ExceptionFtpSettings.h"

//----------------------------------------------------------
void ExceptionFtpSettings::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionFtpSettings *ExceptionFtpSettings::clone() const
{
    return new ExceptionFtpSettings(*this);
}
//----------------------------------------------------------
QString ExceptionFtpSettings::error() const
{
    return m_error;
}
//----------------------------------------------------------
void ExceptionFtpSettings::setError(const QString &newError)
{
    m_error = newError;
}
//----------------------------------------------------------
