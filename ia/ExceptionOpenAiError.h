#ifndef EXCEPTIONOPENAIERROR_H
#define EXCEPTIONOPENAIERROR_H



#include <QException>

class ExceptionOpenAiError : public QException
{
public:
    void raise() const override;
    ExceptionOpenAiError *clone() const override;


    const QString &error() const;
    void setError(const QString &newError);

private:
    QString m_error;
};

#endif // EXCEPTIONOPENAIERROR_H
