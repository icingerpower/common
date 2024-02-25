#ifndef TRANSLATIONNOTDONEEXCEPTION_H
#define TRANSLATIONNOTDONEEXCEPTION_H


#include <QException>

class ExceptionTranslationNotDone : public QException
{
public:
    void raise() const override;
    ExceptionTranslationNotDone *clone() const override;
    QString error() const;

    void setErrorInfo(const QString &text, int position);

    const QString &line() const;
    int position() const;

private:
    QString m_textNotTranslated;
    int m_position;

};

#endif // TRANSLATIONNOTDONEEXCEPTION_H
