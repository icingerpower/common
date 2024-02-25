#ifndef EXCEPTIONTRANSLATIONSAME_H
#define EXCEPTIONTRANSLATIONSAME_H


#include <QException>

class ExceptionTranslationSame : public QException
{
public:
    void raise() const override;
    ExceptionTranslationSame *clone() const override;
    QString error() const;

    void setErrorInfo(
            const int rowIndex,
            const QString &line,
            const QString &langTo1,
            const QString &langTo2);


private:
    int m_rowIndex;
    QString m_line;
    QString m_langTo1;
    QString m_langTo2;
};


#endif // EXCEPTIONTRANSLATIONSAME_H
