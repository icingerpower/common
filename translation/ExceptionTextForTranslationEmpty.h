#ifndef EXCEPTIONTEXTFORTRANSLATIONEMPTY_H
#define EXCEPTIONTEXTFORTRANSLATIONEMPTY_H

#include <QException>

class ExceptionTextForTranslationEmpty : public QException
{
public:
    void raise() const override;
    ExceptionTextForTranslationEmpty *clone() const override;

    QString error() const;
    const QString &textNotTranslated() const;
    void setTextNotTranslated(const QString &newTextNotTranslated);

private:
    QString m_textNotTranslated;

};
#endif // EXCEPTIONTEXTFORTRANSLATIONEMPTY_H
