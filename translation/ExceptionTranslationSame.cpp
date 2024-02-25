#include <QObject>

#include "ExceptionTranslationSame.h"


//----------------------------------------------------------
void ExceptionTranslationSame::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionTranslationSame *ExceptionTranslationSame::clone() const
{
    return new ExceptionTranslationSame(*this);
}
//----------------------------------------------------------
QString ExceptionTranslationSame::error() const
{
    return QObject::tr("Text pasted is similar for 2 different languages starting on line: ")
            + QString::number(m_rowIndex + 1)
            + " - " + m_langTo1 + "/" + m_langTo2 + " - " + m_line;
}
//----------------------------------------------------------
void ExceptionTranslationSame::setErrorInfo(
        const int rowIndex,
        const QString &line,
        const QString &langTo1,
        const QString &langTo2)
{
    m_rowIndex = rowIndex;
    m_line = line;
    m_langTo1 = langTo1;
    m_langTo2 = langTo2;
}
//----------------------------------------------------------
