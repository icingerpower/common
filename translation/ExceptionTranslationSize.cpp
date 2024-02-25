#include <QObject>

#include "ExceptionTranslationSize.h"

//---------------------------------------------------------
void ExceptionTranslationSize::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionTranslationSize *ExceptionTranslationSize::clone() const
{
    return new ExceptionTranslationSize(*this);
}
//----------------------------------------------------------
QString ExceptionTranslationSize::error() const
{
    return QObject::tr("Text pasted doesn't have the right number of line ")
            + QString::number(m_sizeTo)
            + QObject::tr("instead of ") + QString::number(m_sizeFrom);
}
//----------------------------------------------------------
void ExceptionTranslationSize::setErrorInfo(const int sizeFrom, int sizeTo)
{
    m_sizeFrom = sizeFrom;
    m_sizeTo = sizeTo;
}
//----------------------------------------------------------

