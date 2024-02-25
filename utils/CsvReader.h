#ifndef CSVREADER_H
#define CSVREADER_H

#include <QtCore/qstring.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qfile.h>
#include <QString>
#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>
#include <qtextstream.h>
#include <QtCore/qiodevice.h>

#include "CsvHeader.h"

struct DataFromCsv {
    CsvHeader header;
    QList<QStringList> lines;
    QStringList skipedLines;
};

class CsvReader
{
public:
    CsvReader(const QString &fileName,
              QString sep = ",",
              QString guillemetsForString = "",
              bool hasHeader = true,
              QString returnLine = "\n",
              int linesToSkip = 0,
              const QString &encoding = "UTF-8");
    static bool isLinuxReturnLine(QString &fileName);
    static bool isValidHeader(
            const QString &fileName,
            const QStringList &elements,
            const QString &encoding = "UTF-8");
    static CsvReader createExcelCsvRead(const QString &fileName);

    //bool atEndLine(bool inGuill, const QString &element, const QString &bufferNext, const QTextStream &stream) const;
    bool isValidHeader(const QStringList &elements, const QString &encoding = "UTF-8") const;
    bool readAll();
    bool readAllOld();
    bool readSomeLines(int nLines);
    const DataFromCsv *dataRode() const;
    void removeFirstLine();
    QStringList takeFirstLine();
    void clear();

    int linesToSkip() const;
    void setLinesToSkip(int linesToSkip);

private:
    QStringList decodeLine(const QString &line) const;
    QStringList decodeLine(QTextStream &stream, QString &bufferNext) const;
    QString m_fileName;
    QString m_guillemetsForString;
    bool m_hasHeader;
    QString m_sep;
    QString m_returnLine;
    QString m_encoding;
    int m_linesToSkip;
    DataFromCsv m_dataRode;
};


#endif // CSVREADER_H
