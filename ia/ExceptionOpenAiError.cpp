#include "ExceptionOpenAiError.h"

//----------------------------------------
void ExceptionOpenAiError::raise() const
{
    throw *this;
}
//----------------------------------------
ExceptionOpenAiError *ExceptionOpenAiError::clone() const
{
    return new ExceptionOpenAiError(*this);
}
//----------------------------------------
const QString &ExceptionOpenAiError::error() const
{
    return m_error;
}
//----------------------------------------
void ExceptionOpenAiError::setError(const QString &newError)
{
    m_error = newError;
}
//----------------------------------------
