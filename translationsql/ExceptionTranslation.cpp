#include <QObject>

#include "ExceptionTranslation.h"

void ExceptionTranslation::raise() const
{
    throw *this;
}

ExceptionTranslation *ExceptionTranslation::clone() const
{
    return new ExceptionTranslation(*this);
}

const QString &ExceptionTranslation::error() const
{
    return m_error;
}

void ExceptionTranslation::setError(const QString &error)
{
    m_error = error;
}

const QString &ExceptionTranslation::title() const
{
    return m_title;
}

void ExceptionTranslation::setTitle(const QString &newTitle)
{
    m_title = newTitle;
}

