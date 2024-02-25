#include <zlib.h>

#include <QSettings>

#include "ExceptionTranslationNotDone.h"

#include "TrZipManager.h"

//----------------------------------------------------------
TrZipManager::TrZipManager(QObject *parent)
    : TrTextManager(parent)
{
}
//----------------------------------------------------------
QString TrZipManager::_fileNameZip(const QString &langCode) const
{
    return "trans-" + langCode + ".zip";
}
//----------------------------------------------------------
TrZipManager *TrZipManager::instance()
{
    static TrZipManager instance;
    return &instance;
}
//----------------------------------------------------------
TrZipManager *TrZipManager::createInstanceForUnitTest()
{
    return new TrZipManager;
}
//----------------------------------------------------------
void TrZipManager::displayTextTranslated(
        const QString &contained,
        const QString &langCode,
        Qt::CaseSensitivity cs)
{
    int nRowsOrig = m_tableToTranslate.size();
    if (nRowsOrig > 0) {
        beginRemoveRows(QModelIndex(), 0, nRowsOrig-1);
        m_tableToTranslate.clear();
        endRemoveRows();
    }
    QStringList allLangs;
    allLangs.append(m_langFrom);
    allLangs.append(m_langsTo);
    int nLangs = m_langsTo.size() + 1;

    QStringList lines;
    QList<int> toDisplayIndexes;
    QString fileName = _fileNameZip(langCode);
    QString filePath = m_workingDir.filePath(fileName);
    QFile file(filePath);
    int colLang = allLangs.indexOf(langCode);

    readAllLines(lines, langCode);
    auto copyTableToTranslate = m_tableToTranslate;
    for (int i=0; i<lines.size(); ++i) {
        if (lines[i].contains(contained, cs)) {
            toDisplayIndexes << i;
            copyTableToTranslate << QStringList(nLangs);
            copyTableToTranslate[copyTableToTranslate.size()-1][colLang] = lines[i];
        }
    }
    if (copyTableToTranslate.size() > 0) {
        beginInsertRows(QModelIndex(), 0, copyTableToTranslate.size()-1);
        m_tableToTranslate = copyTableToTranslate;
        copyTableToTranslate.clear();
        for (int col=0; col < nLangs; ++col) {
            if (allLangs[col] != langCode) {
                lines.clear();
                readAllLines(lines, allLangs[col]);
                int i=0;
                for (int index : toDisplayIndexes) {
                    m_tableToTranslate[i][col] = lines[index];
                    ++i;
                }
            }
        }
        endInsertRows();
        m_updateMode = true;
    }
}
//----------------------------------------------------------
int TrZipManager::countHowMuchTranslatedAlready() const
{
    QStringList lines;
    readAllLines(lines, m_langFrom);
    return lines.size();
}
//----------------------------------------------------------
void TrZipManager::convertFromText()
{
    auto allLangs = m_langsTo;
    allLangs.insert(0, m_langFrom);
    for (auto itLangCode = allLangs.begin();
         itLangCode != allLangs.end(); ++itLangCode) {
        QString fileNameFrom = _fileNameCsv(*itLangCode);
        QString filePathFrom = m_workingDir.filePath(fileNameFrom);
        QString fileNameTo = _fileNameZip(*itLangCode);
        QString filePathTo = m_workingDir.filePath(fileNameTo);
        QFile fileFrom(filePathFrom);
        if (fileFrom.open(QFile::ReadOnly)) {
            QTextStream streamFrom(&fileFrom);
            QStringList lines(streamFrom.readAll().split("\n"));
            lines.removeAll(QString());
            fileFrom.close();
            writeAllLines(lines, *itLangCode);
            QStringList linesRodeAgain;
            readAllLines(linesRodeAgain, *itLangCode);
            QString lastOrig = lines[linesRodeAgain.size() - 1];
            QString lastOrig2 = lines[linesRodeAgain.size() - 2];
            QString lastNew = linesRodeAgain[linesRodeAgain.size() - 1];
            QString lastNew2 = linesRodeAgain[linesRodeAgain.size() - 2];
            Q_ASSERT(lines.size() == linesRodeAgain.size());
            QFile::remove(filePathFrom);
            /*
            gzFile zipFileTo = gzopen(filePathTo.toLocal8Bit().constData(), "wb");
            if (zipFileTo != nullptr) {
                QString text = lines.join("\n");
                gzwrite(zipFileTo, text.toLocal8Bit().constData(), text.length());
                gzclose(zipFileTo);
            }
            //*/
        }
    }
}
//----------------------------------------------------------
void TrZipManager::save()
{
    if (!m_updateMode) {
        int nLines = m_tableToTranslate.size();
        if (nLines > 0) {
            int nCols = m_tableToTranslate[0].size();
            for (int i=0; i<nLines; ++i) {
                for (int col=1; col<nCols; ++col) {
                    if (m_tableToTranslate[i][col].isEmpty()) {
                        ExceptionTranslationNotDone exception;
                        exception.setErrorInfo(
                                    m_tableToTranslate[i][0], i);
                        exception.raise();
                    }
                }
            }
            QStringList allLangs;
            allLangs.append(m_langFrom);
            allLangs.append(m_langsTo);
            for (int col=0; col<allLangs.size(); ++col) {
                QString langCode = allLangs[col];
                QString fileName = _fileNameZip(langCode);
                QString filePath = m_workingDir.filePath(fileName);
                bool fileExists = QFile::exists(filePath);
                gzFile zipFile = nullptr;
                if (fileExists){
                    zipFile = gzopen(filePath.toLocal8Bit().constData(), "ab");
                } else {
                    zipFile = gzopen(filePath.toLocal8Bit().constData(), "wb");
                }
                if (zipFile != nullptr) {
                    QStringList lines;
                    for (int i=0; i<nLines; ++i) {
                        lines << m_tableToTranslate[i][col];
                    }
                    if (fileExists) {
                        lines[0].insert(0, "\n");
                    }
                    auto textUtf8 = lines.join("\n").toUtf8();
                    gzwrite(zipFile, textUtf8.constData(), textUtf8.length());
                    gzclose(zipFile);
                }
            }
            m_lastTranslations.clear();
            QString lastFilePath = _lastTransSettingFilePath();
            for (int col=1; col<m_langsTo.size(); ++col) {
                QString langCode = allLangs[col];
                QStringList lines;
                for (int i=0; i<nLines; ++i) {
                    lines << m_tableToTranslate[i][col];
                }
                m_lastTranslations[langCode] = lines;
            }
            QSettings settingsLast(lastFilePath, QSettings::IniFormat);
            settingsLast.setValue(KEY_LAST_TRANSLATED,
                                  QVariant::fromValue(m_lastTranslations));

        }
        for (auto itTr = m_tableToTranslate.begin();
             itTr != m_tableToTranslate.end(); ++itTr) {
            m_linesTranslatedAlready << itTr->value(0);
        }
        beginRemoveRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        m_linesToTranslate.clear();
        m_tableToTranslate.clear();
        clearTemporary();
        endRemoveRows();
        _assertSameNumberOfLines(); // TODO remove
    }

}
//----------------------------------------------------------
void TrZipManager::saveUpdated()
{
    if (m_updateMode) {
        QStringList linesFrom;
        readAllLines(linesFrom, m_langFrom);
        QHash<QString, int> textToPosition;
        for (int i=0; i<linesFrom.size(); ++i) {
            textToPosition[linesFrom[i]] = i;
        }
        linesFrom.clear();
        QHash<int, int> editedToPosition;
        for (int i=0; i<m_tableToTranslate.size(); ++i) {
            QString fromText = m_tableToTranslate[i][0];
            int fromPos = textToPosition[fromText];
            editedToPosition[i] = fromPos;
        }
        textToPosition.clear();
        int indCol = 1;
        for (auto langTo = m_langsTo.begin();
             langTo != m_langsTo.end(); ++langTo) {
            if (m_indexesColEditedManually.contains(indCol)) {
                QStringList linesTo;
                readAllLines(linesTo, *langTo);
                int langToCol = m_langsTo.indexOf(*langTo) + 1;
                for (int i=0; i<m_tableToTranslate.size(); ++i) {
                    int posFrom = editedToPosition[i];
                    linesTo[posFrom] = m_tableToTranslate[i][langToCol];
                }
                writeAllLines(linesTo, *langTo);
                /*
                QString fileNameTo = _fileNameCsv(*langTo);
                QString filePathTo = m_workingDir.filePath(fileNameTo);
                QString text = linesTo.join("\n");
                auto zipFile = gzopen(filePathTo.toLocal8Bit().constData(), "wb");
                if (zipFile != nullptr) {
                    gzwrite(zipFile, text.toLocal8Bit().constData(), text.length());
                    gzclose(zipFile);
                }
                //*/
            }
            ++indCol;
        }
        beginRemoveRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        m_linesToTranslate.clear();
        m_tableToTranslate.clear();
        m_indexesColEditedManually.clear();
        clearTemporary();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void TrZipManager::loadTranslatedAlready()
{
    _clear();
    QStringList lines;
    readAllLines(lines, m_langFrom);
    for (auto itLine = lines.begin();
         itLine != lines.end(); ++itLine) {
        m_linesTranslatedAlready << *itLine;
    }
            /*
    QString fileNameFrom = _fileNameZip(m_langFrom);
    QString filePathFrom = m_workingDir.filePath(fileNameFrom);
    gzFile fileZlib = gzopen(filePathFrom.toLocal8Bit().constData(), "rb");
    if (fileZlib != nullptr)
    {
    QFile fileFrom(filePathFrom);
        linesFrom << streamFrom.readAll().split("\n");
        fileFrom.close();
        for (auto lineIt = linesFrom.begin();
             lineIt != linesFrom.end(); ++lineIt) {
            m_linesTranslatedAlready << *lineIt;
        }
    }
            //*/
}
//----------------------------------------------------------
void TrZipManager::readAllLines(
        QStringList &lines, const QString &langCode) const
{
    static char buffer[10240];
    auto sizeBuffer = sizeof(buffer);
    QString fileName = _fileNameZip(langCode);
    QString filePath = m_workingDir.filePath(fileName);
    gzFile fileZlib = gzopen(filePath.toLocal8Bit().constData(), "rb");
    if (fileZlib != nullptr)
    {
        while (gzeof(fileZlib) == 0) {
            gzgets(fileZlib, buffer, sizeBuffer);
            QString line(buffer);
            if (line.endsWith("\n")) {
                line.remove(line.size()-1, 1);
            }
            lines << line;
        }
    }
    gzclose(fileZlib);
    /*
    if (langCode == "it") {
        QString filePathFix = "/home/cedric/Dropbox/ApplicationsData/dropshipping-stores/freelancers/projects/websites/TRANSLATIONS/healing-to-fix/it-1384173-1404172.txt";
        QStringList linesFix;
        QFile file(filePathFix);
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            linesFix = stream.readAll().split("\n");
            for (int i=0; i<linesFix.size(); ++i) {
                lines[1384172 + i] = linesFix[i];
            }
            file.close();
        }
        writeAllLines(lines, "it");
    }
    //*/
}
//----------------------------------------------------------
void TrZipManager::writeAllLines(
        const QStringList &lines, const QString &langCode) const
{
    QString fileName = _fileNameZip(langCode);
    QString filePath = m_workingDir.filePath(fileName);
    gzFile zipFileTo = gzopen(filePath.toLocal8Bit().constData(), "wb");
    if (zipFileTo != nullptr) {
        auto textUtf8 = lines.join("\n").toUtf8();
        gzwrite(zipFileTo, textUtf8.constData(), textUtf8.length());
        gzclose(zipFileTo);
    }
}
//----------------------------------------------------------
void TrZipManager::loadInBufferLangTo(const QString &langTo)
{
    QStringList linesFrom;
    readAllLines(linesFrom, m_langFrom);
    QStringList linesTo;
    readAllLines(linesTo, langTo);
    for (int i=0; i<linesFrom.size(); ++i) {
        m_buffer[linesFrom[i]] = linesTo[i];
    }
}
//----------------------------------------------------------
