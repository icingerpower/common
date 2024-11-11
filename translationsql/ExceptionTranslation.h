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


    const QString &title() const;
    void setTitle(const QString &newTitle);

private:
    QString m_error;
    QString m_title;
};

#endif // EXCEPTIONTRANSLATIONSIZE_H
