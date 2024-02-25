#include "ExceptionFtpUpload.h"


//----------------------------------------------------------
void ExceptionFtpUpload::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionFtpUpload *ExceptionFtpUpload::clone() const
{
    return new ExceptionFtpUpload(*this);
}
//----------------------------------------------------------
QString ExceptionFtpUpload::error() const
{
    return m_errorOutput;
}
//----------------------------------------------------------
void ExceptionFtpUpload::setError(
        QProcess::ProcessError processError,
        const QString &errorOutput)
{
    m_errorOutput = errorOutput;
}
//----------------------------------------------------------
