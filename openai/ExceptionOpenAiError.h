#ifndef EXCEPTIONOPENAIERROR_H
#define EXCEPTIONOPENAIERROR_H



#include <QException>

class ExceptionOpenAiError : public QException
{
public:
    void raise() const override;
    ExceptionOpenAiError *clone() const override;

    QString title() const;
    const QString &error() const;
    void setError(const QString &newError);

private:
    QString m_error;
};

#endif // EXCEPTIONOPENAIERROR_H
