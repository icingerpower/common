#include <QObject>

#include "ExceptionTextForTranslationEmpty.h"

//---------------------------------------------------------
void ExceptionTextForTranslationEmpty::raise() const
{
    throw *this;
}
//---------------------------------------------------------
ExceptionTextForTranslationEmpty
*ExceptionTextForTranslationEmpty::clone() const
{
    return new ExceptionTextForTranslationEmpty(*this);
}
//---------------------------------------------------------
QString ExceptionTextForTranslationEmpty::error() const
{
    return QObject::tr("The following class type is not translated:")
            + m_textNotTranslated;
}
//---------------------------------------------------------
const QString &ExceptionTextForTranslationEmpty::textNotTranslated() const
{
    return m_textNotTranslated;
}
//---------------------------------------------------------
void ExceptionTextForTranslationEmpty::setTextNotTranslated(
        const QString &newTextNotTranslated)
{
    m_textNotTranslated = newTextNotTranslated;
}
//---------------------------------------------------------
