#ifndef EXCEPTIONPROXYPATHNOTSET_H
#define EXCEPTIONPROXYPATHNOTSET_H

#include <QException>


class ExceptionProxyPathNotSet : public QException
{
public:
    void raise() const override;
    ExceptionProxyPathNotSet *clone() const override;

    QString error() const;

    void setError(const QString &newError);

private:
    QString m_error;
};

#endif // EXCEPTIONPROXYPATHNOTSET_H
