#ifndef EXCEPTIONOPENAINOTINITIALIZED_H
#define EXCEPTIONOPENAINOTINITIALIZED_H

#include <QException>

class ExceptionOpenAiNotInitialized : public QException
{
public:
    void raise() const override;
    ExceptionOpenAiNotInitialized *clone() const override;

    QString error() const;

};

#endif // EXCEPTIONOPENAINOTINITIALIZED_H
