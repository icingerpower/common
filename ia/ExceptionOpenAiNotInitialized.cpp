#include <QObject>

#include "ExceptionOpenAiNotInitialized.h"


//----------------------------------------------------------
void ExceptionOpenAiNotInitialized::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionOpenAiNotInitialized *ExceptionOpenAiNotInitialized::clone() const
{
    return new ExceptionOpenAiNotInitialized(*this);
}
//----------------------------------------------------------
QString ExceptionOpenAiNotInitialized::error() const
{
    return QObject::tr("Open Ai has not been initialized");
}
//----------------------------------------------------------
