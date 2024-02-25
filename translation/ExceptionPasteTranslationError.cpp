#include <QObject>

#include "ExceptionPasteTranslationError.h"


//---------------------------------------------------------
void ExceptionPasteTranslationError::raise() const
{
    throw *this;
}
//---------------------------------------------------------
ExceptionPasteTranslationError
*ExceptionPasteTranslationError::clone() const
{
    return new ExceptionPasteTranslationError(*this);
}
//---------------------------------------------------------
void ExceptionPasteTranslationError::setError(const QString &error)
{
    m_error = error;
}
//---------------------------------------------------------
QString ExceptionPasteTranslationError::error() const
{
    return m_error;
}
//---------------------------------------------------------
