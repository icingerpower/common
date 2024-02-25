#include "CsvHeader.h"

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
void CsvHeaderException::raise() const
{
    throw *this;
}
//----------------------------------------------------------
CsvHeaderException *CsvHeaderException::clone() const
{
    return new CsvHeaderException(*this);
}
//----------------------------------------------------------
QStringList CsvHeaderException::columnValuesError() const
{
    return m_columnValuesError;
}
//----------------------------------------------------------
void CsvHeaderException::setColumnValuesError(const QStringList &columnValuesError)
{
    m_columnValuesError = columnValuesError;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
CsvHeader::CsvHeader()
{
}
//----------------------------------------------------------
CsvHeader::CsvHeader(const QStringList &headerElements)
{
    setHeaderElements(headerElements);
}
//----------------------------------------------------------
int CsvHeader::pos(const QStringList &names) const
{
    int keyIndex = 0;
    while (!m_headerPosition.contains(names[keyIndex])) {
        keyIndex++;
        if (keyIndex == names.length()) {
            CsvHeaderException exeption;
            exeption.setColumnValuesError(names);
            exeption.raise();
        }
    }
    return m_headerPosition[names[keyIndex]];
}
//----------------------------------------------------------
int CsvHeader::pos(
        const QStringList &names, const QString &customError) const
{
    int keyIndex = 0;
    while (!m_headerPosition.contains(names[keyIndex])) {
        keyIndex++;
        if (keyIndex == names.length()) {
            CsvCustomerHeaderException exeption;
            exeption.setError(customError);
            exeption.raise();
        }
    }
    return m_headerPosition[names[keyIndex]];
}
//----------------------------------------------------------
bool CsvHeader::contains(const QString &name) const
{
    return m_headerPosition.contains(name);
}
//----------------------------------------------------------
int CsvHeader::pos(const QString &name) const
{
    int pos = -1;
    if (m_headerPosition.contains(name)) {
        pos = m_headerPosition[name];
    } else {
        CsvHeaderException exeption;
        exeption.setColumnValuesError({name});
        exeption.raise();
    }
    return pos;
}
//----------------------------------------------------------
int CsvHeader::pos(const QString &name, const QString &customError) const
{
    int pos = -1;
    if (m_headerPosition.contains(name)) {
        pos = m_headerPosition[name];
    } else {
        CsvCustomerHeaderException exeption;
        exeption.setError(customError);
        exeption.raise();
    }
    return pos;
}
//----------------------------------------------------------
QStringList CsvHeader::getHeaderElements() const
{
    return m_headerElements;
}
//----------------------------------------------------------
void CsvHeader::setHeaderElements(const QStringList &headerElements)
{
    m_headerPosition.clear();
    m_headerElements << headerElements;
    for (int i=0; i<headerElements.size(); ++i) {
        m_headerPosition[headerElements[i].trimmed()] = i;
    }
}
//----------------------------------------------------------
void CsvCustomerHeaderException::raise() const
{
    throw *this;
}
//----------------------------------------------------------
CsvCustomerHeaderException *CsvCustomerHeaderException::clone() const
{
    return new CsvCustomerHeaderException(*this);
}
//----------------------------------------------------------
QString CsvCustomerHeaderException::error() const
{
    return m_error;
}
//----------------------------------------------------------
void CsvCustomerHeaderException::setError(const QString &error)
{
    m_error = error;
}
//----------------------------------------------------------
void CsvHeader::clear()
{
    m_headerElements.clear();
    m_headerPosition.clear();
    m_colVariantMap.clear();
}
//----------------------------------------------------------
