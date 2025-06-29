#include <QFile>
#include <QMap>

#include "CsvReader.h"


//----------------------------------------------------------
QPair<QString, QString> CsvReader::guessColStringSeps(const QString &filePath)
{
    QPair<QString, QString> colAndString;
    QFile file(filePath);
    if (file.open(QFile::ReadOnly))
    {
        QTextStream stream{&file};
        QString line = stream.readLine();
        QMap<int, QString> countSep;
        QStringList seps{"\t", ";", ","};
        for (const auto &sep : seps)
        {
            int count = line.count(sep);
            countSep[count] = sep;
            colAndString.first = countSep.last();
        }
        QString line2 = stream.readLine();
        if (line2.size() > 1)
        {
            line = line2;
        }
        //int nElements = line2.count(colAndString.first);
        int nGuillemets = line2.count("\"");
        if (nGuillemets > 3)
        {
            colAndString.second = "\"";
        }
        file.close();
    }
    return colAndString;
}
//----------------------------------------------------------
CsvReader::CsvReader(const QString &fileName,
        QString sep,
        QString guillemetsForString,
        bool hasHeader,
        QString returnLine,
        int linesToSkip,
        const QString &encoding)
{
    m_fileName = fileName;
    m_sep = sep;
    m_hasHeader = hasHeader;
    m_guillemetsForString = guillemetsForString;
    m_returnLine = returnLine;
    m_linesToSkip = linesToSkip;
    m_encoding = encoding;
}
//----------------------------------------------------------
bool CsvReader::isLinuxReturnLine(QString &fileName)
{
    bool is = false;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString line = stream.readLine();
        file.seek(0);
        QString firstData = stream.read(line.length()*2+2);
        is = !firstData.contains("\r\n");
        file.close();
    }
    return is;
}
//----------------------------------------------------------
bool CsvReader::isValidHeader(
        const QString &fileName,
        const QStringList &elements,
        const QString &encoding)
{
    bool is = false;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        #if QT_VERSION <= 0x050000
        if (encoding != "UTF-8") {
            stream.setCodec(encoding.toStdString().c_str());
        }
        #else
        Q_UNUSED(encoding);
        #endif
        QString line = stream.readLine();
        is = true;
        for (auto element : elements) {
            if (!line.contains(element)){
                is = false;
                break;
            }
        }
        file.close();
    }
    return is;
}
//----------------------------------------------------------
CsvReader CsvReader::createExcelCsvRead(const QString &fileName)
{
    QString sep = ";";
    QString guill = "";
    CsvReader reader(fileName, sep, guill, true, "\r\n");
    return reader;
}
//----------------------------------------------------------
/*
bool CsvReader::atEndLine(
        bool inGuill,
        const QString &element,
        const QString &bufferNext,
        const QTextStream &stream) const
{
    return stream.atEnd()
            || (!inGuill && bufferNext == "\n");
}
//*/
//----------------------------------------------------------
bool CsvReader::isValidHeader(
        const QStringList &elements, const QString &encoding) const
{
    bool is = CsvReader::isValidHeader(m_fileName, elements, encoding);
    return is;
}
//----------------------------------------------------------
bool CsvReader::readAll()
{
    m_dataRode.lines.clear();
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        QString bufferNext;
        #if QT_VERSION <= 0x050000
        if (m_encoding != "UTF-8") {
            stream.setCodec(m_encoding.toStdString().c_str());
        }
        #endif
        while (m_linesToSkip - m_dataRode.skipedLines.size() > 0
               && !stream.atEnd()) {
            QString line = stream.readLine();
            m_dataRode.skipedLines << line;
        }
        if (m_hasHeader) {
            QStringList headerElements = decodeLine(stream, bufferNext);
            m_dataRode.header = CsvHeader(headerElements);
        }
        while (!stream.atEnd()) {
            QStringList elementsLine = decodeLine(stream, bufferNext);
            m_dataRode.lines << elementsLine;
        }
        file.close();
        return true;
    }
    return false;
}
//----------------------------------------------------------
bool CsvReader::readAllOld()
{
    m_dataRode.lines.clear();
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        #if QT_VERSION <= 0x050000
        if (m_encoding != "UTF-8") {
            stream.setCodec(m_encoding.toStdString().c_str());
        }
        #endif
        QString line;
        while (m_linesToSkip - m_dataRode.skipedLines.size() > 0
               && !stream.atEnd()) {
            QString line = stream.readLine();
            m_dataRode.skipedLines << line;
        }
        if (m_hasHeader) {
            QString line = stream.readLine();
            // TODO here it can't work in return in csv line
            QStringList headerElements = decodeLine(line);
            m_dataRode.header = CsvHeader(headerElements);
        }
        do {
            line = stream.readLine();
            line = line.replace("\"\"", "''");
            if (!line.isEmpty()) { /// It will skip empty lines in middle of file
                QStringList decodedLine = decodeLine(line);
                m_dataRode.lines << decodedLine;
            }
        } while (!stream.atEnd());
        file.close();
        return true;
    }
    return false;
}
// TODO handle space in cells => Instead of reading some lines I should read some bytes little by little
//----------------------------------------------------------
bool CsvReader::readSomeLines(int nLines)
{
    m_dataRode.lines.clear();
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        #if QT_VERSION <= 0x050000
        if (m_encoding != "UTF-8") {
            stream.setCodec(m_encoding.toStdString().c_str());
        }
        #endif
        while (m_linesToSkip - m_dataRode.skipedLines.size() > 0
               && !stream.atEnd()) {
            QString line = stream.readLine();
            m_dataRode.skipedLines << line;
        }
        QString line = stream.readLine();
        if (m_hasHeader) {
            QStringList headerElements = decodeLine(line);
            m_dataRode.header = CsvHeader(headerElements);
        }
        int curLine = 0;
        while (curLine < nLines) {
            line = stream.readLine();
            line = line.replace("\"\"", "''");
            if (line.isEmpty()) {
                break;
            } else {
                QStringList decodedLine = decodeLine(line);
                m_dataRode.lines << decodedLine;
            }
            ++curLine;
        }
        file.close();
        return true;
    }
    return false;
}
//----------------------------------------------------------
const DataFromCsv *CsvReader::dataRode() const
{
    return &m_dataRode;
}
//----------------------------------------------------------
void CsvReader::removeFirstLine()
{
    if (m_dataRode.lines.size() > 0) {
        m_dataRode.lines.removeAt(0);
    }
}
//----------------------------------------------------------
QStringList CsvReader::takeFirstLine()
{
    QStringList elements = m_dataRode.lines.takeAt(0);
    return elements;
}
//----------------------------------------------------------
QStringList CsvReader::decodeLine(const QString &line) const
{
    QStringList elements;
    if (m_guillemetsForString.isEmpty()) {
        elements = line.split(m_sep);
    } else {
        int lenSep = m_sep.length();
        int lenGuill = m_guillemetsForString.length();
        QString lineCopy = line.trimmed();
        bool end = false;
        while(!end) {
            if (lineCopy.startsWith(m_guillemetsForString)) {
                lineCopy.remove(0, lenGuill);
                int indexEnd = lineCopy.indexOf(m_guillemetsForString);
                QString element = lineCopy.left(indexEnd);
                elements << element;
                lineCopy.remove(0, indexEnd + lenSep + lenGuill);
            } else {
                int indexEnd = lineCopy.indexOf(m_sep);
                if (indexEnd == -1) {
                    indexEnd = lineCopy.size()-1;
                }
                QString element = lineCopy.left(indexEnd);
                elements << element;
                lineCopy.remove(0, indexEnd + lenSep);
            }
            end = lineCopy.isEmpty();
        }
    }
    return elements;
}
/*
//----------------------------------------------------------
QStringList CsvReader::decodeLine(QTextStream &stream) const
{
    QStringList elements;
    QString element;
    bool inGuill = false;
    int sizeBuffer = 3 + m_sep.size() + m_guillemetsForString.size();
    QString buffer;
    for (int i=0; i<sizeBuffer; ++i) {
        buffer += "x";
    }
    while(!stream.atEnd()) {
        QString charStream = stream.read(1);
        element += charStream;
        buffer += charStream;
        buffer.remove(0, 1);
        //int streamPos = 0;
        //if (element.endsWith("camile")) {
        if (m_guillemetsForString == "\""
                && element.endsWith(m_guillemetsForString) && !stream.atEnd()) {
            /// We handle "" which means not a string separator
            QString nextChar = stream.read(1);
            if (nextChar == "\"") {
                element.remove(element.size() - 1, 1);
                element +* "''";
            } else {
                stream.seek(stream.pos() - 1);
            }
        }
        if (!inGuill && !m_guillemetsForString.isEmpty() && element.startsWith(m_guillemetsForString)) {
            inGuill = true;
        } else if (inGuill
                   && element.size() > m_guillemetsForString.size()
                   && element.endsWith(m_guillemetsForString)) {
            int guillSize = m_guillemetsForString.size();
            element.remove(0, guillSize);
            element.remove(element.size() - guillSize, guillSize);
            elements << element;
            element.clear();
            inGuill = false;
            if (!stream.atEnd()) {
                int posStream = stream.pos();
                QString nextChars = stream.read(m_sep.size());
                if (nextChars != m_sep) {
                    stream.seek(posStream);
                }
            }
        } else if (!inGuill
                   && (element.endsWith("\r\n")
                       || element.endsWith("\n"))) {
            int sizeReturn = element.endsWith("\r\n") ? 2 : 1;
            element.remove(element.size() - sizeReturn, sizeReturn);
            if (element.isEmpty()) {
                int lenSep = m_sep.size();
                int lenTotal = lenSep + sizeReturn;
                QString lastChars = buffer.mid(sizeBuffer - lenTotal, lenSep);
                if (lastChars == m_sep) {
                    elements << ""; /// We only add empty if before it was a separator "end"\n
                }
            } else {
                elements << element;
            }
            return elements;
        } else if (!inGuill && element.endsWith(m_sep)) {
            element.remove(element.size() - m_sep.size(), m_sep.size());
            elements << element;
            element.clear();
        }
    }
    return elements;
}
//*/
//*
//----------------------------------------------------------
QStringList CsvReader::decodeLine(
        QTextStream &stream, QString &bufferNext) const
{
    QStringList elements;
    QString element;
    bool inGuill = false;
    int sizeBuffer = 3 + m_sep.size() + m_guillemetsForString.size();
    QString buffer;
    for (int i=0; i<sizeBuffer; ++i) {
        buffer += "x";
    }
    if (!stream.atEnd() && bufferNext.isEmpty()) {
        bufferNext = stream.read(1);
    }
    bool guilChar = false;
    while(!bufferNext.isEmpty()) {
        QString charStream = bufferNext;
        if (!stream.atEnd()) {
            bufferNext = stream.read(1);
        } else {
            bufferNext = "";
        }
        element += charStream;
        buffer += charStream;
        buffer.remove(0, 1);
        guilChar = false;
        if (element.endsWith("\"")) {
            if (bufferNext == "\"") {
                if (m_guillemetsForString != "\"") {
                    guilChar = true;
                } else {
                    if (inGuill) {
                        guilChar = true;
                    }
                }
            } else if (element.endsWith("\"\"")) {
                if (m_guillemetsForString == "\"") {
                    int nGuill = element.count("\"");
                    if (nGuill % 2 != 0 && element != "\"\"") {
                        guilChar = true;
                    }
                } else {
                    guilChar = true;
                }
            }
        }
        if (!inGuill && !m_guillemetsForString.isEmpty() && element.startsWith(m_guillemetsForString)) {
            inGuill = true;
        } else if (inGuill
                   && element.size() > m_guillemetsForString.size()
                   && element.endsWith(m_guillemetsForString)
                   && !(m_guillemetsForString == "\""
                        && guilChar)) {
            //if (m_guillemetsForString == "\""
            //&& bufferNext == "\"") {
            //element.remove(element.size() - 1, 1);
            //} else {
            int guillSize = m_guillemetsForString.size();
            element.remove(0, guillSize);
            element.remove(element.size() - guillSize, guillSize);
            element.replace("\"\"", "\"");
            elements << element;
            element.clear();
            inGuill = false;
            if (!stream.atEnd()) {
                if (bufferNext == m_sep) {
                    if (!stream.atEnd()) {
                        bufferNext = stream.read(1);
                    } else {
                        bufferNext = "";
                    }
                }
            }
            //}
        } else if (!inGuill
                   && (element.endsWith("\r\n")
                       || element.endsWith("\n"))) {
            int sizeReturn = element.endsWith("\r\n") ? 2 : 1;
            element.remove(element.size() - sizeReturn, sizeReturn);
            if (element.isEmpty()) {
                int lenSep = m_sep.size();
                int lenTotal = lenSep + sizeReturn;
                QString lastChars = buffer.mid(sizeBuffer - lenTotal, lenSep);
                if (lastChars == m_sep) {
                    elements << ""; /// We only add empty if before it was a separator "end"\n
                }
            } else {
                elements << element;
            }
            return elements;
        } else if (!inGuill && element.endsWith(m_sep)) {
            element.remove(element.size() - m_sep.size(), m_sep.size());
            elements << element;
            element.clear();
        } else if (!inGuill && bufferNext.isEmpty()) {
            elements << element;
            element.clear();
        }
    }
    return elements;
}
//*/
//----------------------------------------------------------
int CsvReader::linesToSkip() const
{
    return m_linesToSkip;
}
//----------------------------------------------------------
void CsvReader::setLinesToSkip(int linesToSkip)
{
    m_linesToSkip = linesToSkip;
}
//----------------------------------------------------------
void CsvReader::clear()
{
    m_dataRode.lines.clear();
    m_dataRode.skipedLines.clear();
    m_dataRode.header.clear();
}

QStringList CsvReader::readLastLine() const
{
    QStringList elements;
    QFile file(m_fileName);
    if (file.open(QIODevice::ReadOnly) && file.size() > 0) {

        // Start from the end of the file and move backward
        file.seek(file.size() - 1);
        QByteArray lastLine;
        char ch;

        while (file.pos() > 0) {
            file.read(&ch, 1);

            if (ch == '\n' && lastLine.size() > 1) {
                break;  // Reached the start of the last line
            }

            lastLine.prepend(ch);  // Prepend character to the result
            file.seek(file.pos() - 2);  // Move one character back
        }

        // For the very last line (if no newline at the end of file)
        if (file.pos() == 0) {
            file.seek(0);
            QByteArray firstLine = file.readLine();
            lastLine.prepend(firstLine);
        }
        elements = decodeLine(QString::fromUtf8(lastLine));
    }
    return elements;
}
//----------------------------------------------------------
