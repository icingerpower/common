#ifndef EXCEPTIONFTPUPLOAD_H
#define EXCEPTIONFTPUPLOAD_H


#include <QException>
#include <QProcess>

class ExceptionFtpUpload : public QException
{
public:
    void raise() const override;
    ExceptionFtpUpload *clone() const override;

    QString error() const;
    void setError(QProcess::ProcessError processError,
                  const QString &errorOutput);

private:
    QProcess::ProcessError m_errorProcess;
    QString m_errorOutput;
};

#endif // EXCEPTIONFTPUPLOAD_H
