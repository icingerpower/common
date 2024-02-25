#ifndef CSVHEADER_H
#define CSVHEADER_H

#include <QtCore/qhash.h>
#include <QtCore/qexception.h>
#include <QtCore/qstringlist.h>

class CsvHeaderException : public QException
{
public:
    void raise() const override;
    CsvHeaderException *clone() const override;

    QStringList columnValuesError() const;
    void setColumnValuesError(const QStringList &columnValuesError);

private:
    QStringList m_columnValuesError;
};

class CsvCustomerHeaderException : public QException
{
public:
    void raise() const override;
    CsvCustomerHeaderException *clone() const override;


    QString error() const;
    void setError(const QString &error);

private:
    QString m_error;
};

class CsvHeader
{
public:
    CsvHeader();
    CsvHeader(const QStringList &headerElements);
    int pos(const QString &name) const;
    int pos(const QString &name, const QString &customError) const;
    int pos(const QStringList &names) const;
    int pos(const QStringList &names, const QString &customError) const;
    bool contains(const QString &name) const;

    QStringList getHeaderElements() const;
    void setHeaderElements(const QStringList &headerElements);
    void clear();

private:
    QStringList m_headerElements;
    QHash<QString, int> m_headerPosition;
    QHash<QString, QString> m_colVariantMap;
};

#endif // CSVHEADER_H
