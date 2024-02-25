#include <QSettings>
#include <QHash>

#include "ExceptionPasteTranslationError.h"
#include "ExceptionTranslationNotDone.h"
#include "ExceptionTranslationSize.h"
#include "ExceptionTranslationSame.h"

#include "TrTextManager.h"

//*
//----------------------------------------------------------
QString TrTextManager::KEY_LAST_TRANSLATED = "last-translation";
QString TrTextManager::KEY_TEMPORARY_TRANSLATION = "temporary-translation";
QString TrTextManager::KEY_TEMPORARY_TRANSLATION_BACKUP = "temporary-translation-backup";
//----------------------------------------------------------
TrTextManager::TrTextManager(QObject *parent)
    : QAbstractTableModel(parent)
{

}
//----------------------------------------------------------
TrTextManager *TrTextManager::instance()
{
    static TrTextManager instance;
    return &instance;

}
//----------------------------------------------------------
TrTextManager *TrTextManager::createInstanceForUnitTest()
{
    return new TrTextManager;
}
//----------------------------------------------------------
QSet<int> TrTextManager::addSplitText(QStringList &lines, const QString &text)
{
    lines = text.split(". ");
    for (int i=0; i<lines.size()-1; ++i) {
        lines[i] += ".";
    }
    QStringList chars = {"! ", "? ", " - ", "\n"};
    for (auto charIt = chars.begin(); charIt != chars.end(); ++charIt) {
        for (int i = lines.size()-1; i>=0; --i) {
            if (lines[i].contains(*charIt)) {
                QString temp = lines[i];
                lines.removeAt(i);
                QStringList elements = temp.split(*charIt);
                for (auto elIt = elements.rbegin(); elIt != elements.rend(); ++elIt) {
                    Q_ASSERT(!elIt->isEmpty());
                    lines.insert(i, *elIt);
                }
                QString charWitoutEndSpace = *charIt;
                if (charWitoutEndSpace.size() > 1) {
                    charWitoutEndSpace.remove(charWitoutEndSpace.size()-1, 1);
                }
                if (!charWitoutEndSpace.isEmpty()) {
                    for (int j=i; j<i+elements.size()-1; ++j) {
                        lines[j] += charWitoutEndSpace;
                    }
                }
            }
        }
    }
    QSet<int> indexesWithReturnLines;
    for (int i=0; i<lines.size(); ++i) {
        if (lines[i].endsWith("\n")) {
            indexesWithReturnLines << i;
            lines[i].remove(lines[i].size()-1, 1);
        }
    }
    return indexesWithReturnLines;
}
//----------------------------------------------------------
QDir TrTextManager::workingDir() const
{
    return m_workingDir;
}
//----------------------------------------------------------
bool TrTextManager::isUpdateMode() const
{
    return m_updateMode;
}
//----------------------------------------------------------
void TrTextManager::init(
        const QDir &workingDir,
        const QString &langCodeFrom,
        const QStringList &langCodesTo,
        const QString &langNameFrom,
        const QStringList &langNamesTo)
{
    m_workingDir = workingDir;
    m_workingDir.mkpath(".");
    m_langFrom = langCodeFrom;
    m_langsTo = langCodesTo;
    m_langsTo.removeAll(m_langFrom);
    m_langNameFrom = langNameFrom;
    m_langNamesTo = langNamesTo;
    m_langNamesTo.removeAll(langNameFrom);
}
//----------------------------------------------------------
QString TrTextManager::getLangCode(int index) const
{
    if (index == 0) {
        return m_langFrom;
    }
    return m_langsTo[index - 1];
}
//----------------------------------------------------------
int TrTextManager::countHowMuchTranslatedAlready() const
{
    QStringList linesFrom;
    readAllLines(linesFrom, m_langFrom);
    int nLines = linesFrom.size();
    /*
    QString fileNameFrom = "trans-" + m_langFrom + ".csv";
    QString filePathFrom = m_workingDir.filePath(fileNameFrom);
    int nLines = 0;
    QFile fileFrom(filePathFrom);
    if (fileFrom.open(QFile::ReadOnly)) {
        QTextStream streamFrom(&fileFrom);
        QStringList linesFrom = streamFrom.readAll().split("\n");
        nLines = linesFrom.size();
        fileFrom.close();
    }
    //*/
    return nLines;
}
//----------------------------------------------------------
void TrTextManager::loadSomeLines(QStringList &lines, int rowIndex, int colIndex, int nRows)
{
    QStringList linesTemp;
    readAllLines(linesTemp, getLangCode(colIndex));
    int nMax = rowIndex + nRows;
    lines.reserve(nRows);
    for (int i=rowIndex; i<nMax; ++i) {
        lines << linesTemp[i];
    }
}
//----------------------------------------------------------
void TrTextManager::writeSomeLines(QStringList &lines, int rowIndex, int colIndex)
{
    QStringList linesTemp;
    QString langCode = getLangCode(colIndex);
    readAllLines(linesTemp, langCode);
    for (int i=0; i<lines.size(); ++i) {
        linesTemp[rowIndex + i] = lines[i];
    }
    writeAllLines(linesTemp, langCode);
}
//----------------------------------------------------------
QVariant TrTextManager::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList header = [this]() -> QStringList {
            QStringList _header;
            _header << m_langFrom + " - " + m_langNameFrom;
            for (int i=0; i<m_langsTo.size(); ++i) {
                _header << m_langsTo[i] + " - " + m_langNamesTo[i];
            }
            return _header;
        }();
        return header[section];
    } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return QString::number(section + 1);
    }
    return QVariant();
}
//----------------------------------------------------------
int TrTextManager::rowCount(const QModelIndex &) const
{
    return m_tableToTranslate.size();
}
//----------------------------------------------------------
int TrTextManager::columnCount(const QModelIndex &) const
{
    return m_langsTo.size() + 1;
}
//----------------------------------------------------------
QVariant TrTextManager::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_tableToTranslate[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool TrTextManager::setData(
        const QModelIndex &index,
        const QVariant &value,
        int role)
{
    if (role == Qt::EditRole
            && data(index, Qt::DisplayRole) != value) {
        QString string = value.toString();
        if (!string.isEmpty()) {
            if (m_updateMode) {
                m_indexesColEditedManually << index.column();
            }
            m_tableToTranslate[index.row()][index.column()]
                    = value.toString();
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags TrTextManager::flags(
        const QModelIndex &index) const
{
    if (index.column() > 0) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
//----------------------------------------------------------
void TrTextManager::displayTextTranslated(
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
    readAllLines(lines, langCode);
    //QString fileName = _fileNameCsv(langCode);
    //QString filePath = m_workingDir.filePath(fileName);
    //QFile file(filePath);
    int colLang = allLangs.indexOf(langCode);
    //if (file.open(QFile::ReadOnly)) {
    //QTextStream streamFrom(&file);
    //lines << streamFrom.readAll().split("\n");
    //file.close();
    for (int i=0; i<lines.size(); ++i) {
        if (lines[i].contains(contained, cs)) {
            toDisplayIndexes << i;
            m_tableToTranslate << QStringList(nLangs);
            m_tableToTranslate[m_tableToTranslate.size()-1][colLang] = lines[i];
        }
    }
    //}
    if (m_tableToTranslate.size() > 0) {
        for (int col=0; col < nLangs; ++col) {
            if (allLangs[col] != langCode) {
                lines.clear();
                readAllLines(lines, allLangs[col]);
                //QString fileName = _fileNameCsv(allLangs[col]);
                //QString filePath = m_workingDir.filePath(fileName);
                //QFile file(filePath);
                //if (file.open(QFile::ReadOnly)) {
                //QTextStream streamFrom(&file);
                //lines = streamFrom.readAll().split("\n");
                //file.close();
                int i=0;
                for (int index : toDisplayIndexes) {
                    m_tableToTranslate[i][col] = lines[index];
                    ++i;
                }
                //}
            }
        }
        beginInsertRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        endInsertRows();
        m_updateMode = true;
    }
}
//----------------------------------------------------------
void TrTextManager::clearTextTranslated()
{
    _clear();
}
//----------------------------------------------------------
void TrTextManager::pasteTranslatedText(const QString &langCode,
        const QStringList &lines, bool force)
{
    int indLangTo = m_langsTo.indexOf(langCode) + 1;
    if (m_tableToTranslate.size()
            != lines.size()) {
        ExceptionTranslationSize exception;
        exception.setErrorInfo(m_tableToTranslate.size(), lines.size());
        exception.raise();
    }
    if (lines == m_lastTranslations[langCode]) {
        ExceptionPasteTranslationError exception;
        exception.setError(
                    QObject::tr("The text pasted is the same as the previous round"));
        exception.raise();
    }
    if (!force) {
        int i=0;
        int nSameMax = 10;
        int nSame = 0;
        int nCols = columnCount();
        for (auto itLine = m_tableToTranslate.begin();
             itLine != m_tableToTranslate.end();
             ++itLine) {
            if (itLine->value(indLangTo).isEmpty()
                    && !itLine->value(indLangTo).trimmed().isEmpty()) {
                ExceptionPasteTranslationError exception;
                exception.setError(
                            QObject::tr("The following line doesn't have a translation")
                            + ": " + QString::number(i+1) + " - "
                            + itLine->value(0));
                exception.raise();
            }
            bool hasSame = false;
            int col = 0;
            for (col = 0; col<nCols; ++col) {
                if (col != indLangTo) {
                    if (itLine->value(col) == lines[i]) {
                        hasSame = true;
                        break;
                    }
                }
            }
            if (hasSame) {
                ++nSame;
                if (nSame == nSameMax) {
                    ExceptionTranslationSame exception;
                    int rowIndex = i-nSameMax+1;
                    QString langTo1 = getLangCode(col);
                    QString langTo2 = getLangCode(indLangTo);
                    exception.setErrorInfo(
                                rowIndex,
                                m_tableToTranslate[rowIndex][0],
                            langTo1, langTo2);
                    exception.raise();
                }
            } else {
                nSame = 0;
            }
            ++i;
        }
    }
    for (int i=0; i<m_tableToTranslate.size(); ++i) {
        m_tableToTranslate[i][indLangTo] = lines[i].trimmed();
        const auto &text = m_tableToTranslate[i][indLangTo];
        Q_ASSERT(!text.contains("\n"));
    }
    if (lines.size() > 0) {
        saveBackup();
        emit dataChanged(
                    index(0, indLangTo),
                    index(lines.size()-1, indLangTo),
                    QList<int>() << Qt::DisplayRole);
    }
}
//----------------------------------------------------------
void TrTextManager::pasteTranslatedTextForcing(
        const QString &langCode, const QStringList &lines)
{
    pasteTranslatedText(langCode, lines, true);
}
//----------------------------------------------------------
void TrTextManager::displayTextToTranslate()
{
    m_updateMode = false;
    int nRowsOrig = m_tableToTranslate.size();
    if (nRowsOrig > 0) {
        beginRemoveRows(QModelIndex(), 0, nRowsOrig);
        m_tableToTranslate.clear();
        endRemoveRows();
    }
    /*
    QString fileName = _fileName(m_langFrom);
    QString filePath = m_workingDir.filePath(fileName);
    QFile file(filePath);
    QSet<QString> linesTranslatedSet;
    if (file.open(QFile::ReadOnly)) {
        QTextStream streamFrom(&file);
        QStringList linesTranslated = streamFrom.readAll().split("\n");
        file.close();
        for (auto itTrans = linesTranslated.begin();
             itTrans != linesTranslated.end(); ++itTrans) {
            linesTranslatedSet << *itTrans;
        }
        linesTranslated.clear();
    }
    //*/
    for (auto it = m_linesToTranslate.begin();
         it!= m_linesToTranslate.end(); ++it) {
        //if (!linesTranslatedSet.contains(*it)) {
        m_tableToTranslate << QStringList(columnCount());
        m_tableToTranslate[m_tableToTranslate.size()-1][0] = *it;
        //}
    }
    if (m_tableToTranslate.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        endInsertRows();
    }
    if (m_lastTranslations.isEmpty()) {
        QSettings settings(_lastTransSettingFilePath(), QSettings::IniFormat);
        if (settings.contains(KEY_LAST_TRANSLATED)) {
            m_lastTranslations = settings.value(
                        KEY_LAST_TRANSLATED)
                    .value<QHash<QString, QStringList>>();
        }
    }

}
//----------------------------------------------------------
QStringList TrTextManager::getDisplayedTextToTranslate() const
{
    QStringList texts;
    for (auto it=m_tableToTranslate.begin();
         it != m_tableToTranslate.end(); ++it) {
        texts << it->value(0);
    }
    return texts;
}
//----------------------------------------------------------
bool TrTextManager::isAllTextAddedTranslated() const
{
    return m_tableToTranslate.size() == 0;
}
//----------------------------------------------------------
void TrTextManager::saveTemporary() const
{
    QSettings settings;
    settings.setValue(KEY_TEMPORARY_TRANSLATION,
                      QVariant::fromValue(m_tableToTranslate));
}
//----------------------------------------------------------
void TrTextManager::loadTemporary()
{
    _loadTemporary(KEY_TEMPORARY_TRANSLATION);
}
//----------------------------------------------------------
void TrTextManager::clearTemporary()
{
    QSettings settings;
    if (settings.contains(KEY_TEMPORARY_TRANSLATION)) {
        settings.remove(KEY_TEMPORARY_TRANSLATION);
    }
}
//----------------------------------------------------------
void TrTextManager::saveBackup() const
{
    QSettings settings;
    settings.setValue(KEY_TEMPORARY_TRANSLATION_BACKUP,
                      QVariant::fromValue(m_tableToTranslate));
}
//----------------------------------------------------------
void TrTextManager::loadBackup()
{
    _loadTemporary(KEY_TEMPORARY_TRANSLATION_BACKUP);
}
//----------------------------------------------------------
void TrTextManager::clearBackup()
{
    QSettings settings;
    if (settings.contains(KEY_TEMPORARY_TRANSLATION_BACKUP)) {
        settings.remove(KEY_TEMPORARY_TRANSLATION_BACKUP);
    }
}
//----------------------------------------------------------
void TrTextManager::convertFromText()
{
}
//----------------------------------------------------------
void TrTextManager::_loadTemporary(const QString &settingKey)
{
    _clear();
    QSettings settings;
    m_tableToTranslate
            = settings.value(settingKey)
            .value<QList<QStringList>>();
    for (auto it=m_tableToTranslate.begin(); it!= m_tableToTranslate.end(); ++it) {
        m_linesToTranslate << it->value(0);
    }
    if (m_tableToTranslate.size() > 0) {
        beginInsertRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        //QString fileNameFrom = _fileName(m_langFrom);
        //QString filePathFrom = m_workingDir.filePath(fileNameFrom);
        endInsertRows();
    }
}
//----------------------------------------------------------
QString TrTextManager::_fileNameCsv(const QString &langCode) const
{
    return "trans-" + langCode + ".csv";
}
//----------------------------------------------------------
void TrTextManager::readAllLines(
        QStringList &lines, const QString &langCode) const
{
    QString fileName = _fileNameCsv(langCode);
    QString filePath = m_workingDir.filePath(fileName);
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QTextStream streamFrom(&file);
        lines = streamFrom.readAll().split("\n");
        file.close();
    }
}
//----------------------------------------------------------
void TrTextManager::writeAllLines(
        const QStringList &lines, const QString &langCode) const
{
    QString fileName = _fileNameCsv(langCode);
    QString filePath = m_workingDir.filePath(fileName);
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << lines.join("\n");
        file.close();
    }
}
//----------------------------------------------------------
void TrTextManager::save()
{
    if (!m_updateMode) {
        int nLines = m_tableToTranslate.size();
        if (nLines > 0) {
            int nCols = m_tableToTranslate[0].size();
            //int nTransSame = 0;
            //int nTransSameMax = 20;
            for (int i=0; i<nLines; ++i) {
                //bool hasEqualsTrans = false;
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
                QString fileName = _fileNameCsv(langCode);
                QString filePath = m_workingDir.filePath(fileName);
                QFile file(filePath);
                bool addReturnLine = false;
                if (QFile::exists(filePath)) {
                    addReturnLine = true;
                }
                if (file.open(QFile::WriteOnly | QFile::Append)) {
                    QStringList lines;
                    for (int i=0; i<nLines; ++i) {
                        lines << m_tableToTranslate[i][col];
                    }
                    QTextStream stream(&file);
                    if (addReturnLine){
                        lines[0].insert(0, "\n");
                    }
                    stream << lines.join("\n");
                    file.close();
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
void TrTextManager::saveUpdated()
{
    if (m_updateMode) {
        //QString fileNameFrom = _fileNameCsv(m_langFrom);
        //QString filePathFrom = m_workingDir.filePath(fileNameFrom);
        //QFile fileFrom(filePathFrom);
        QStringList linesFrom;
        readAllLines(linesFrom, m_langFrom);
        /*
        if (fileFrom.open(QFile::ReadOnly)) {
            QTextStream streamFrom(&fileFrom);
            linesFrom << streamFrom.readAll().split("\n");
            fileFrom.close();
        }
        //*/
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
                QString fileNameTo = _fileNameCsv(*langTo);
                QString filePathTo = m_workingDir.filePath(fileNameTo);
                readAllLines(linesTo, *langTo);
                /*
                QFile fileTo(filePathTo);
                if (fileTo.open(QFile::ReadOnly)) {
                    QTextStream streamTo(&fileTo);
                    linesTo = streamTo.readAll().split("\n");
                    fileTo.close();
                }
                //*/
                int langToCol = m_langsTo.indexOf(*langTo) + 1;
                for (int i=0; i<m_tableToTranslate.size(); ++i) {
                    int posFrom = editedToPosition[i];
                    linesTo[posFrom] = m_tableToTranslate[i][langToCol];
                }
                writeAllLines(linesTo, *langTo);
                /*
                if (fileTo.open(QFile::WriteOnly)) {
                    QTextStream streamTo(&fileTo);
                    streamTo << linesTo.join("\n");
                    fileTo.close();
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
bool TrTextManager::addTextToTranslate(
        const QString &text, int maxLines)
{
    Q_ASSERT(!text.trimmed().isEmpty());
    Q_ASSERT(!m_updateMode);
    QStringList lines;
    addSplitText(lines, text);
    //Q_ASSERT(!text.contains("\n"));
    //auto lines = splitText(text);
    if (lines.size() > 1) {
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            QString TEMP = *it;
            if (it->trimmed().isEmpty()) {
                qDebug() << text;
            }
            Q_ASSERT(!it->trimmed().isEmpty()); // It means there is something like an extra space at the end
            Q_ASSERT(!it->contains("\n")); // It means there is something like an extra space at the end
            if (!addTextToTranslate(*it, maxLines)) {
                return false;
            }
        }
    } else {
        if (!m_linesTranslatedAlready.contains(text)) {
            m_linesToTranslate << text;
        }
    }
    return m_linesToTranslate.size() < maxLines;
}
//----------------------------------------------------------
int TrTextManager::nLinesToTranslate() const
{
    return m_linesToTranslate.size();
}
//----------------------------------------------------------
void TrTextManager::loadTranslatedAlready()
{
    _clear();
    //QString fileNameFrom = _fileNameCsv(m_langFrom);
    //QString filePathFrom = m_workingDir.filePath(fileNameFrom);
    //QFile fileFrom(filePathFrom);
    QStringList linesFrom;
    readAllLines(linesFrom, m_langFrom);
    /*
    if (fileFrom.open(QFile::ReadOnly)) {
        QTextStream streamFrom(&fileFrom);
        linesFrom << streamFrom.readAll().split("\n");
        fileFrom.close();
    }
    //*/
    for (auto lineIt = linesFrom.begin();
         lineIt != linesFrom.end(); ++lineIt) {
        m_linesTranslatedAlready << *lineIt;
    }
}
//----------------------------------------------------------
void TrTextManager::loadInBufferLangTo(const QString &langTo)
{
    _clear();
    m_langToBuffer = langTo;
    /*
    QString fileNameFrom = "trans-" + m_langFrom + ".csv";
    QString filePathFrom = m_workingDir.filePath(fileNameFrom);
    //*/
    QString fileNameTo = "trans-" + langTo + ".csv";
    QString filePathTo = m_workingDir.filePath(fileNameTo);
    QStringList linesFrom;
    QStringList linesTo;
    readAllLines(linesFrom, m_langFrom);
    /*
    QFile fileFrom(filePathFrom);
    if (fileFrom.open(QFile::ReadOnly)) {
        QTextStream streamFrom(&fileFrom);
        linesFrom << streamFrom.readAll().split("\n");
        fileFrom.close();
    }
    //*/
    QFile fileTo(filePathTo);
    if (fileTo.open(QFile::ReadOnly)) {
        QTextStream streamTo(&fileTo);
        linesTo << streamTo.readAll().split("\n");
        while (linesTo.last().trimmed().isEmpty()) {
            linesTo.takeLast();
        }
        fileTo.close();
    }
    Q_ASSERT(linesFrom.size() == linesTo.size());
    for (int i=0; i<linesFrom.size(); ++i) {
        m_buffer[linesFrom[i]] = linesTo[i];
    }
}
//----------------------------------------------------------
void TrTextManager::analysePotentialDuplicates(
        QMap<int, QStringList> &linesWithDuplicates)
{
    QStringList linesFrom;
    readAllLines(linesFrom, m_langFrom);
    int nCols = m_langsTo.size() + 1;
    QHash<QString, int> langCodeToIndex;
    langCodeToIndex[m_langFrom] = 0;
    for (auto itLang = m_langsTo.begin();
         itLang != m_langsTo.end(); ++itLang) {
        langCodeToIndex[*itLang] = langCodeToIndex.size();
        QStringList linesTo;
        readAllLines(linesTo, *itLang);
        QMultiHash<QString, int> textToPositions;
        QStringList textDublicate;
        for (int i=0; i<linesTo.size(); ++i) {
            textToPositions.insert(linesTo[i], i);
            int count = textToPositions.count(linesTo[i]);
            if (count > 1) {
                textDublicate << linesTo[i];
            }
        }
        for (auto itText = textDublicate.begin();
             itText != textDublicate.end(); ++itText) {
            auto indexes = textToPositions.values(*itText);
            for (auto itIndex = indexes.begin();
                 itIndex != indexes.end(); ++itIndex) {
                int rowIndex = *itIndex;
                if (!linesWithDuplicates.contains(rowIndex)) {
                    linesWithDuplicates.insert(rowIndex, QStringList(nCols));
                    linesWithDuplicates[rowIndex][0] = linesFrom[rowIndex];
                }
                int colIndex = langCodeToIndex[*itLang];
                linesWithDuplicates[rowIndex][colIndex] = *itText;
            }
        }
    }

    _clear();
}
//----------------------------------------------------------
void TrTextManager::trOrAffect(
        QString &textTo,
        const QString &textFrom,
        bool translate)
{
    if (translate) {
        getTr(textTo, textFrom);
    } else {
        textTo = textFrom;
    }
}
//----------------------------------------------------------
void TrTextManager::addTrIfNeeded(
        QString &textToAddTo,
        const QString &textFrom,
        bool translate)
{
    if (translate) {
        addTr(textToAddTo, textFrom);
    } else {
        textToAddTo += textFrom;
    }
}
//----------------------------------------------------------
void TrTextManager::addTrIfNeeded(
        QList<QVariant> &variants,
        const QString &textFrom,
        bool translate)
{
    if (translate) {
        QVariant variant;
        getTr(variant, textFrom);
        variants << variant;
    } else {
        variants << textFrom;
    }
}
//----------------------------------------------------------
QString TrTextManager::getTr(const QString &text) const
{
    QString value;
    getTr(value, text);
    return value;
}
//----------------------------------------------------------
void TrTextManager::getTr(
        QString &toUpdate,
        const QString &text) const
{
    QStringList lines;
    auto indexesWithReturnLines = addSplitText(lines, text);
    if (lines.size() > 0) {
        int i = 0;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            Q_ASSERT(m_buffer.contains(*it));
            *it = m_buffer[*it];
            if (indexesWithReturnLines.contains(i)) {
                *it += "\n";
            }
            ++i;
        }
        toUpdate = lines.join(" ");
        return;
    }
    toUpdate = m_buffer[lines[0]];
}
//----------------------------------------------------------
void TrTextManager::getTr(QVariant &toUpdate, const QString &text) const
{
    QStringList lines;
    auto indexesWithReturnLines = addSplitText(lines, text);
    if (lines.size() > 0) {
        int i = 0;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            Q_ASSERT(m_buffer.contains(*it));
            *it = m_buffer[*it];
            if (indexesWithReturnLines.contains(i)) {
                *it += "\n";
            }
            ++i;
        }
        toUpdate = lines.join(" ");
        return;
    }
    toUpdate = m_buffer[lines[0]];
}
//----------------------------------------------------------
void TrTextManager::addTr(QString &toUpdate, const QString &text)
{
    QStringList lines;
    auto indexesWithReturnLines = addSplitText(lines, text);
    if (lines.size() > 0) {
        int i = 0;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            Q_ASSERT(m_buffer.contains(*it));
            *it = m_buffer[*it];
            if (indexesWithReturnLines.contains(i)) {
                *it += "\n";
            }
            ++i;
        }
        toUpdate += lines.join(" ");
    } else {
        toUpdate += m_buffer[lines[0]];
    }
}
//----------------------------------------------------------
void TrTextManager::addTrIfDone(
        QString &toUpdate, const QString &text)
{
    QStringList lines;
    auto indexesWithReturnLines = addSplitText(lines, text);
    if (lines.size() > 0) {
        int i = 0;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            toUpdate += m_buffer.value(*it, QString());
            if (indexesWithReturnLines.contains(i)) {
                *it += "\n";
            }
            ++i;
        }
        toUpdate += lines.join(" ");
    } else {
        toUpdate += m_buffer.value(lines[0], QString());
    }
}
//----------------------------------------------------------
void TrTextManager::addTr(QList<QVariant> &variants, const QString &text)
{
    QStringList lines;
    auto indexesWithReturnLines = addSplitText(lines, text);
    if (lines.size() > 0) {
        int i = 0;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            Q_ASSERT(m_buffer.contains(*it));
            *it = m_buffer[*it];
            if (indexesWithReturnLines.contains(i)) {
                *it += "\n";
            }
            ++i;
        }
        variants << lines.join(" ");
    } else {
        variants << m_buffer[lines[0]];
    }

}
//----------------------------------------------------------
void TrTextManager::addTrIfDone(QList<QVariant> &variants, const QString &text)
{
    QStringList lines;
    auto indexesWithReturnLines = addSplitText(lines, text);
    if (lines.size() > 0) {
        int i = 0;
        QString trans;
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            trans += m_buffer.value(*it, QString());
            if (indexesWithReturnLines.contains(i)) {
                *it += "\n";
            }
            ++i;
        }
        trans += lines.join(" ");
        variants << trans;
    } else {
        variants << m_buffer.value(lines[0], QString());
    }
}
//----------------------------------------------------------
QString TrTextManager::_lastTransSettingFilePath() const
{
    return m_workingDir.filePath("lastTrans.ini");
}
//----------------------------------------------------------
void TrTextManager::_assertSameNumberOfLines()
{
    QStringList allLangs;
    allLangs.append(m_langFrom);
    allLangs.append(m_langsTo);
    int lastN = -1;
    for (int col=0; col<allLangs.size(); ++col) {
        QString langCode = allLangs[col];
        QString fileName = _fileNameCsv(langCode);
        QString filePath = m_workingDir.filePath(fileName);
        QStringList lines;
        readAllLines(lines, langCode);
        QFile file(filePath);
        //if (file.open(QFile::ReadOnly)) {
        //QTextStream stream(&file);
        //QStringList lines = stream.readAll().split("\n");
        //file.close();
        int newN = lines.size();
        Q_ASSERT(lastN == -1 || lastN == newN);
        lastN = newN;
        //}
    }
}
//----------------------------------------------------------
void TrTextManager::_clear()
{
    if (m_tableToTranslate.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_tableToTranslate.size()-1);
        m_tableToTranslate.clear();
        endRemoveRows();
    }
    m_buffer.clear();
    m_linesToTranslate.clear();
    m_linesTranslatedAlready.clear();
    m_indexesColEditedManually.clear();
    m_updateMode = false;
}
//----------------------------------------------------------

//*/
