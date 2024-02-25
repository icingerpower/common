#ifndef EXCEPTIONPASTETRANSLATIONERROR_H
#define EXCEPTIONPASTETRANSLATIONERROR_H

#include <QException>

class ExceptionPasteTranslationError : public QException
{
public:
    void raise() const override;
    ExceptionPasteTranslationError *clone() const override;
    void setInfos(int line, const QString &text, const QString &translation);
    void setError(const QString &error);
    QString error() const;

private:
    QString m_textNotTranslated;
    QString m_error;
};

#endif // EXCEPTIONPASTETRANSLATIONERROR_H
