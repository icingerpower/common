#ifndef EXCEPTIONTRANSLATIONSIZE_H
#define EXCEPTIONTRANSLATIONSIZE_H

#include <QException>

class ExceptionTranslationSize : public QException
{
public:
    void raise() const override;
    ExceptionTranslationSize *clone() const override;
    QString error() const;

    void setErrorInfo(const int sizeFrom, int sizeTo);

    const QString &line() const;
    int position() const;

private:
    int m_sizeFrom;
    int m_sizeTo;

};

#endif // EXCEPTIONTRANSLATIONSIZE_H
