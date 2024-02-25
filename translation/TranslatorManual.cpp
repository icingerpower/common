#include <QSettings>
#include <QFileInfo>
#include <QMimeData>
#include <QFile>
#include <QTextStream>

#include "TranslatorManual.h"

QString TranslatorManual::TODO = "TODO";
//----------------------------------------------------------
TranslatorManual::TranslatorManual(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_showAll = false;
    _loadTranslations();
}
//----------------------------------------------------------
TranslatorManual *TranslatorManual::instance()
{
    static TranslatorManual instance;
    return &instance;
}
//----------------------------------------------------------
void TranslatorManual::_saveTranslations(bool onlyTodo) const
{
    auto dir = workingDir();
    if (!onlyTodo) {
        for (auto itFrom = m_translations.begin();
             itFrom != m_translations.end();
             ++itFrom) {
            for (auto itTo = itFrom.value().begin();
                 itTo != itFrom.value().end();
                 ++itTo) {
                QStringList lines;
                for (auto itText = itTo.value().begin();
                     itText != itTo.value().end();
                     ++itText) {
                    if (itText.value() != TODO) {
                        QString line = QStringList(
                        {itFrom.key(), itTo.key(), itText.key(), itText.value()}).join(";;;");
                        lines << line;
                    }
                }
                QString fileName = _fileName(itFrom.key(), itTo.key());
                QString filePath = dir.filePath(fileName);
                QFile file(filePath);
                if (file.open(QFile::WriteOnly)) {
                    QTextStream stream(&file);
                    stream << lines.join("\n");
                    file.close();
                }
            }
        }
    }
    QStringList linesTODO;
    for (auto values : m_valuesToTranslate) {
        if (values.last() != TODO) {
            QString line = values.join(";;;");
            linesTODO << line;
        }
    }
    QString filePathTodo = dir.filePath(_fileNameTODO());
    QFile file(filePathTodo);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream << linesTODO.join("\n");
        file.close();
    }
}
//----------------------------------------------------------
QString TranslatorManual::_fileName(const QString &from, const QString &to) const
{
    return "trans-" + from + "-" + to + ".txt";
}
//----------------------------------------------------------
QString TranslatorManual::_fileNameTODO() const
{
    return "trans-TODO.txt";
}
//----------------------------------------------------------
void TranslatorManual::_loadTranslations()
{
    if (m_valuesToTranslate.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_valuesToTranslate.size()-1);
        m_valuesToTranslate.clear();
        endRemoveRows();
    }
    m_translations.clear();
    auto fileInfos = workingDir().entryInfoList(QStringList() << "trans-*.txt", QDir::Files);
    for (auto fileInfo : fileInfos) {
        QFile file(fileInfo.filePath());
        if (file.open(QFile::ReadOnly)) {
            QStringList fileNameElements = fileInfo.baseName().split("-");
            QString to = fileNameElements.takeLast();
            QString from = fileNameElements.takeLast();
            _addFromToIfNeeded(from, to);
            QTextStream stream(&file);
            QString line = stream.readLine();
            while (!line.isEmpty()) {
                QStringList elements = line.split(";;;");
                m_translations[from][to][elements[2]] = elements[3];
                line = stream.readLine();
            }
            file.close();
        }
    }
    QString filePathTodo = workingDir().filePath(_fileNameTODO());
    QFile file(filePathTodo);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        QString line = stream.readLine();
        while (!line.isEmpty()) {
            line = stream.readLine();
            QStringList elements = line.split(";;;");
            _addFromToIfNeeded(elements[0], elements[1]);
            m_translations[elements[0]][elements[1]][elements[2]] = elements[3];
            m_valuesToTranslate << elements;
        }
        file.close();
        if (m_valuesToTranslate.size() > 0) {
            beginInsertRows(QModelIndex(), 0, m_valuesToTranslate.size()-1);
            endInsertRows();
        }
    }
}
//----------------------------------------------------------
void TranslatorManual::_addFromToIfNeeded(const QString &from, QString &to)
{
    if (!m_translations.contains(from)) {
        m_translations[from] = QHash<QString, QHash<QString, QString>>();
        m_translations[from][to] = QHash<QString, QString>();
    } else if (!m_translations[from].contains(to)) {
        m_translations[from][to] = QHash<QString, QString>();
    }
}
//----------------------------------------------------------
QString TranslatorManual::getTranslation(
        const QString &text, const QString &from, const QString &to)
{
    if (from == to) {
        return text;
    }
    if (m_translations.contains(from)) {
        if (m_translations[from].contains(to)) {
            if (m_translations[from][to].contains(text)) {
                return m_translations[from][to][text];
            }
        } else {
            m_translations[from][to] = QHash<QString, QString>();
        }
    } else {
        m_translations[from] = QHash<QString, QHash<QString, QString>>();
        m_translations[from][to] = QHash<QString, QString>();
    }
    m_translations[from][to][text] = TODO;
    int pos;
    for (pos=0; pos<m_valuesToTranslate.size(); ++pos) {
        if (m_valuesToTranslate[pos][0] == from
                && m_valuesToTranslate[pos][1] == to) {
            break;
        }
    }
    QStringList values = {from, to, text, TODO};
    beginInsertRows(QModelIndex(), pos, pos);
    m_valuesToTranslate.insert(pos, values);
    endInsertRows();
    //_saveTranslations(true);
    return TODO;
}
//----------------------------------------------------------
QDir TranslatorManual::workingDir() const
{
    QSettings settings;
    return QDir(settings.value(KEY_TRANSLATOR_MANUAL_DIR,
                               QFileInfo(settings.fileName()).dir().absolutePath()).toString());
}
//----------------------------------------------------------
void TranslatorManual::setWorkingDir(const QDir &dir)
{
    QSettings settings;
    settings.setValue(KEY_TRANSLATOR_MANUAL_DIR, dir.absolutePath());
    _loadTranslations();
}
//----------------------------------------------------------
void TranslatorManual::setShowAll(bool show)
{
    if (m_showAll != show) {
        m_showAll = show;
        int nRowsBefore = m_valuesToTranslate.size();
        if (show) {
            for (auto itFrom = m_translations.begin();
                 itFrom != m_translations.end();
                 ++itFrom) {
                for (auto itTo = itFrom.value().begin();
                     itTo != itFrom.value().end();
                     ++itTo) {
                    for (auto itText = itTo.value().begin();
                         itText != itTo.value().end();
                         ++itText) {
                        if (itText.value() != TODO) {
                            m_valuesToTranslate << QStringList({itFrom.key(), itTo.key(), itText.key(), itText.value()});
                        }
                    }
                }
            }
            int nRowsNow = m_valuesToTranslate.size();
            if (nRowsNow > nRowsBefore) {
                beginInsertRows(QModelIndex(), nRowsBefore, nRowsNow-1);
                endInsertRows();
            }
        } else {
            for (int i=m_valuesToTranslate.size()-1; i>=0; ++i) {
                if (m_valuesToTranslate[i][3] != TODO) {
                    m_valuesToTranslate.removeAt(i);
                }
            }
            int nRowsNow = m_valuesToTranslate.size();
            if (nRowsNow > nRowsBefore) {
                beginRemoveRows(QModelIndex(), 0, nRowsBefore-1);
                endRemoveRows();
                beginInsertRows(QModelIndex(), 0, nRowsNow-1);
                endInsertRows();
            }
        }
    }
}
//----------------------------------------------------------
void TranslatorManual::save()
{
    if (!m_showAll) {
        //int nLinesBefore = m_valuesToTranslate.size();
        QList<int> indexToDelete;
        int currentIndex = 0;
        for (auto values : m_valuesToTranslate) {
            if (values.last() != TODO) {
                indexToDelete << currentIndex;
                _addFromToIfNeeded(values[0], values[1]);
                m_translations[values[0]][values[1]][values[2]] = values[3];
            }
            ++currentIndex;
        }
        for (auto it = indexToDelete.rbegin();
             it != indexToDelete.rend();
             ++it) {
            beginRemoveRows(QModelIndex(), *it, *it);
            m_valuesToTranslate.removeAt(*it);
            endRemoveRows();
        }
        _saveTranslations();
        /*
        int nLinesNow = m_valuesToTranslate.size()-1;
        if (nLinesBefore > 0 && nLinesNow != nLinesBefore) {
            beginRemoveRows(QModelIndex(), 0, nLinesBefore-1);
            endRemoveRows();
            if (nLinesNow > 0) {
                beginInsertRows(QModelIndex(), 0, nLinesNow-1);
                endInsertRows();
            }
        }
        //*/
    } else {
        for (auto values : m_valuesToTranslate) {
            if (values.last() != TODO) {
                _addFromToIfNeeded(values[0], values[1]);
                m_translations[values[0]][values[1]][values[2]] = values[3];
            }
        }
        _saveTranslations();
    }
}
//----------------------------------------------------------
void TranslatorManual::paste(
        QModelIndexList indexes, const QString &text)
{
    QStringList lines = text.split("\n");
    if (indexes.size() > 0 && lines.size() > 0) {
        int min = indexes.first().row();
        int max = min-1;
        for (auto line : lines) {
            ++max;
            m_valuesToTranslate[max][3] = line;
        }
        /*
    while (indexes.size() > 0 && lines.size() > 0) {
        auto index = indexes.takeFirst();
        if (index.column() == 3) {
            QString line = lines.takeFirst();
            m_valuesToTranslate[index.row()][index.column()] = line;
            min = qMin(min, index.row());
            max = qMax(max, index.row());
        }
    }
    while (lines.size() > 0 && max+1 < rowCount()) {
        QString line = lines.takeFirst();
        ++max;
        m_valuesToTranslate[max][3] = line;
    }
    //*/
        emit dataChanged(index(min, 3),
                         index(max, 3));
    }
}
//----------------------------------------------------------
void TranslatorManual::exportTranslations(const QDir &dir) const
{
    for (auto it1 = m_translations.begin();
         it1 != m_translations.end();
         ++it1) {
        for (auto it2 = it1.value().begin();
             it2 != it1.value().end();
             ++it2) {
            QStringList keywordsToSave;
            int nWordsToSave = 0;
            int maxWords = 50000;
            for (auto itTxt = it2.value().begin();
                 itTxt != it2.value().end();
                 ++itTxt) {
                if (itTxt.value() == TODO) {
                    ++nWordsToSave;
                    keywordsToSave << itTxt.key();
                    if (nWordsToSave + 1 % maxWords == 0) {
                        QString baseName = "tr-" + it1.key() + "-" + it2.key() + "-" + QString::number(nWordsToSave / maxWords);
                        QString fileName = baseName + ".html";
                        QString filePath = dir.filePath(fileName);
                        _generateHtml(fileName, keywordsToSave, it1.key());
                        keywordsToSave.clear();
                    }
                }
            }
            if (keywordsToSave.size() > 0) {
                QString baseName = "tr-" + it1.key() + "-" + it2.key() + "-" + QString::number(nWordsToSave / maxWords);
                QString fileName = baseName + ".html";
                QString filePath = dir.filePath(fileName);
                _generateHtml(fileName, keywordsToSave, it1.key());
            }
        }
    }
}
//----------------------------------------------------------
void TranslatorManual::_generateHtml(const QString &filePath,
        const QStringList &keywords, const QString &langSource) const
{
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        QString text = "<!DOCTYPE html><html lang=\"" + langSource
                + "\">"
                  "<head><meta charset=\"UTF-8\">"
                  "<style type=\"text/css\">"
                  "p {margin-bottom: 0;  margin-top: 0;} "
                  "</style></head>"
                  "<body>\n<p>"
                + keywords.join("</p>\n<p>")
                + "</p>\n</body></html>";
        stream << text;
        file.close();
    }
}
//----------------------------------------------------------
void TranslatorManual::importTranslations(const QDir &dir)
{
    auto fileInfos = dir.entryInfoList(QStringList() << "*.txt", QDir::Files);
    for (auto fileInfo : fileInfos) {
        QString baseName = fileInfo.baseName();
        QString htmlName = baseName + ".html";
        QFile htmlFile(dir.filePath(htmlName));
        if (htmlFile.open(QFile::ReadOnly)) {
            QStringList fileElements = baseName.split("-");
            QString from = fileElements[1];
            QString to = fileElements[2];
            QTextStream stream(&htmlFile);
            stream.readLine();
            QStringList keywordsFrom = stream.readAll().split("</p>\n<p>");
            keywordsFrom[0] = keywordsFrom[0].split("<p>")[1];
            keywordsFrom.last() = keywordsFrom.last().split("</p>")[0];
            htmlFile.close();
            QFile translatedFile(dir.filePath(fileInfo.fileName()));
            if (translatedFile.open(QFile::ReadOnly)) {
                QTextStream streamTr(&translatedFile);
                QString line = streamTr.readLine();
                QStringList keywordsTo;
                while (!streamTr.atEnd()) {
                    keywordsTo << line.trimmed();
                    line = streamTr.readLine();
                }
                keywordsTo << line.trimmed();
                if (keywordsFrom.size() != keywordsTo.size()) {
                    // TODO raise exception
                }
                for (int i=0; i<keywordsFrom.size(); ++i) {
                    m_translations[from][to][keywordsFrom[i]] = keywordsTo[i];
                }
                translatedFile.close();
                //TODO update m_valuesToTranslate
            }
        }
    }
}
//----------------------------------------------------------
/*
bool TranslatorManual::canDropMimeData(
        const QMimeData *data,
        Qt::DropAction action,
        int row,
        int column,
        const QModelIndex &) const
{
    (void) row;
    if (action == Qt::CopyAction
            && column == 3
            && (data->hasHtml() || data->hasText())){
        return true;
    }
    return false;
}
//----------------------------------------------------------
bool TranslatorManual::dropMimeData(
        const QMimeData *data,
        Qt::DropAction action,
        int row,
        int column,
        const QModelIndex &)
{
    if (action == Qt::CopyAction
            && column == 3) {
        bool hasText = data->hasText();
        bool hasHtml = data->hasText();
        return true;
    }
    return false;
}
//*/
//----------------------------------------------------------
QVariant TranslatorManual::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QStringList values = {"Lang From", "Lang To", "Text", "Text translated"};
        return values[section];
    } else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return QString::number(section + 1);
    }
    return QVariant();
}
//----------------------------------------------------------
int TranslatorManual::rowCount(const QModelIndex &) const
{
    return m_valuesToTranslate.size();
}
//----------------------------------------------------------
int TranslatorManual::columnCount(const QModelIndex &) const
{
    return 4;
}
//----------------------------------------------------------
QVariant TranslatorManual::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_valuesToTranslate[index.row()][index.column()];
    }
    return QVariant();
}
//----------------------------------------------------------
bool TranslatorManual::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (value != m_valuesToTranslate[index.row()][index.column()]) {
            m_valuesToTranslate[index.row()][index.column()] = value.toString();
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------
Qt::ItemFlags TranslatorManual::flags(const QModelIndex &index) const
{
    if (index.column() == 2) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    } else if (index.column() == 3) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled;
}
//----------------------------------------------------------
