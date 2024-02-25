#ifndef EXCEPTIONFTPSETTINGS_H
#define EXCEPTIONFTPSETTINGS_H

#include <QException>

class ExceptionFtpSettings : public QException
{
public:
    void raise() const override;
    ExceptionFtpSettings *clone() const override;

    QString error() const;

    void setError(const QString &newError);

private:
    QString m_error;
};


#endif // EXCEPTIONFTPSETTINGS_H
