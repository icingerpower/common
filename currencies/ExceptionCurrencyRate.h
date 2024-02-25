#ifndef EXCEPTIONCURRENCYRATE_H
#define EXCEPTIONCURRENCYRATE_H

#include <QtCore/qexception.h>

class ExceptionCurrencyRate : public QException
{
public:
    void raise() const override;
    ExceptionCurrencyRate *clone() const override;

    QString url() const;
    void setUrl(const QString &error);

private:
    QString m_urlError;

};

#endif // EXCEPTIONCURRENCYRATE_H
