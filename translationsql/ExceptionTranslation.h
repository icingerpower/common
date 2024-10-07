#ifndef EXCEPTIONTRANSLATIONSIZE_H
#define EXCEPTIONTRANSLATIONSIZE_H

#include <QException>

class ExceptionTranslation : public QException
{
public:
    void raise() const override;
    ExceptionTranslation *clone() const override;
    const QString &error() const;

    void setError(const QString &error);


private:
    QString m_error;
};

#endif // EXCEPTIONTRANSLATIONSIZE_H
