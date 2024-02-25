#include <QObject>

#include "ExceptionTranslationNotDone.h"

//---------------------------------------------------------
void ExceptionTranslationNotDone::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionTranslationNotDone *ExceptionTranslationNotDone::clone() const
{
    return new ExceptionTranslationNotDone(*this);
}
//----------------------------------------------------------
QString ExceptionTranslationNotDone::error() const
{
    return QObject::tr("Text not translated in position ")
            + QString::number(m_position) + ": " + m_textNotTranslated;
}
//----------------------------------------------------------
void ExceptionTranslationNotDone::setErrorInfo(
        const QString &text, int position)
{
    m_textNotTranslated = text;
    m_position = position;
}
//----------------------------------------------------------
const QString &ExceptionTranslationNotDone::line() const
{
    return m_textNotTranslated;
}
//----------------------------------------------------------
int ExceptionTranslationNotDone::position() const
{
    return m_position;
}
//----------------------------------------------------------
