
#include "types.h"


//----------------------------------------
QDataStream & operator << (
        QDataStream &stream, const QList<QStringList> &listOfStringList)
{
    int nLines = listOfStringList.size();
    int nColumns = 0;
    if (nLines> 0) {
        nColumns = listOfStringList[0].size();
    }
    stream << QString::number(nLines) + "-" + QString::number(nColumns);
    for (auto itLine=listOfStringList.begin(); itLine!=listOfStringList.end(); ++itLine) {
        for (auto itElement = itLine->begin(); itElement != itLine->end(); ++itElement) {
            stream << *itElement;
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream, QList<QStringList> &listOfStringList)
{
    QString sizeString;
    stream >> sizeString;
    QStringList sizeInfo = sizeString.split("-");
    int nLines = sizeInfo[0].toInt();
    int nColumns = sizeInfo[1].toInt();
    for (int i=0; i<nLines; ++i) {
        QStringList stringList;
        for (int j=0; j<nColumns; ++j) {
            QString value;
            stream >> value;
            stringList << value;
        }
        listOfStringList << stringList;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream, const QList<QList<QVariant>> &listOfVariantList)
{
    int nLines = listOfVariantList.size();
    int nColumns = 0;
    if (nLines> 0) {
        nColumns = listOfVariantList[0].size();
    }
    stream << QString::number(nLines) + "-" + QString::number(nColumns);
    for (auto itLine=listOfVariantList.begin(); itLine!=listOfVariantList.end(); ++itLine) {
        for (auto itElement = itLine->begin(); itElement != itLine->end(); ++itElement) {
            stream << *itElement;
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream, QList<QList<QVariant>> &listOfVariantList)
{
    QString sizeString;
    stream >> sizeString;
    QStringList sizeInfo = sizeString.split("-");
    int nLines = sizeInfo[0].toInt();
    int nColumns = sizeInfo[1].toInt();
    for (int i=0; i<nLines; ++i) {
        QList<QVariant> variantList;
        for (int j=0; j<nColumns; ++j) {
            QVariant value;
            stream >> value;
            variantList << value;
        }
        listOfVariantList << variantList;
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QList<QSize> &sizes)
{
    stream << int(sizes.size());
    for (const auto &size : sizes)
    {
        stream << size.width();
        stream << size.height();
    }
    return stream;

}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QList<QSize> &sizes)
{
    int nValues{0};
    stream >> nValues;
    int width{0};
    int height{0};
    for (int i=0; i<nValues; ++i)
    {
        stream >> width;
        stream >> height;
        sizes << QSize{width, height};
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QList<QDate> &dates)
{
    stream << int(dates.size());
    for (const auto &date : dates)
    {
        stream << date;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QList<QDate> &dates)
{
    int nValues{0};
    stream >> nValues;
    QDate date;
    for (int i=0; i<nValues; ++i)
    {
        stream >> date;
        dates << date;
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QMap<QDate, int> &mapDateInt)
{
    const QList<QDate> &dates = mapDateInt.keys();
    stream << dates;
    const auto &values = mapDateInt.values();
    for (const auto &value : values)
    {
        stream << value;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QMap<QDate, int> &mapDateInt)
{
    QList<QDate> dates;
    stream >> dates;
    for (const auto &date : dates)
    {
        stream >> mapDateInt[date];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QMap<QDate, double> &mapDateDouble)
{
    const QList<QDate> &dates = mapDateDouble.keys();
    stream << dates;
    const auto &values = mapDateDouble.values();
    for (const auto &value : values)
    {
        stream << value;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QMap<QDate, double> &mapDateDouble)
{
    QList<QDate> dates;
    stream >> dates;
    for (const auto &date : dates)
    {
        stream >> mapDateDouble[date];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QSet<QString> &set)
{
    int nElements = set.size();
    stream << QString::number(nElements);
    for (const auto &string : set) {
        stream << string;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QSet<QString> &set)
{
    QString nElementsString;
    stream >> nElementsString;
    int nElements = nElementsString.toInt();
    for (int i=0; i<nElements; ++i) {
        QString value;
        stream >> value;
        set << value;
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream
                           , const QHash<int, QDateTime> &hashOfInt_dateTime)
{
    QList<int> keys = hashOfInt_dateTime.keys();
    stream << keys;
    for (auto itPair=hashOfInt_dateTime.begin(); itPair!=hashOfInt_dateTime.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream
                           , QHash<int, QDateTime> &hashOfInt_dateTime)
{
    QList<int> keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> hashOfInt_dateTime[*itKey];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QString> &hashOfStrings)
{
    QStringList keys = hashOfStrings.keys();
    stream << keys;
    for (auto itPair=hashOfStrings.begin(); itPair!=hashOfStrings.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QString> &hashOfStrings)
{
    QStringList keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> hashOfStrings[*itKey];
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, bool> &hashOfStringBool)
{
    QStringList keys = hashOfStringBool.keys();
    stream << keys;
    for (auto itPair=hashOfStringBool.begin(); itPair!=hashOfStringBool.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, bool> &hashOfStringBool)
{
    QStringList keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> hashOfStringBool[*itKey];
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QMap<QString, bool> &mapOfStringBool)
{
    QStringList keys = mapOfStringBool.keys();
    stream << keys;
    for (auto itPair=mapOfStringBool.begin(); itPair!=mapOfStringBool.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QMap<QString, bool> &mapOfStringBool)
{
    QStringList keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> mapOfStringBool[*itKey];
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QVariant> &hashOfStringVariant) {
    QStringList keys = hashOfStringVariant.keys();
    stream << keys;
    for (auto itPair=hashOfStringVariant.begin();
         itPair!=hashOfStringVariant.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QVariant> &hashOfStringVariant) {
    QStringList keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> hashOfStringVariant[*itKey];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QHash<QString, QDateTime> &hashOfStringDateTime)
{
    const QStringList &keys = hashOfStringDateTime.keys();
    stream << keys;
    for (auto itPair=hashOfStringDateTime.begin();
         itPair!=hashOfStringDateTime.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QHash<QString, QDateTime> &hashOfStringDateTime)
{
    QStringList keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> hashOfStringDateTime[*itKey];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QDate> &hashOfStringDate) {
    const QStringList &keys = hashOfStringDate.keys();
    stream << keys;
    for (auto itPair=hashOfStringDate.begin();
         itPair!=hashOfStringDate.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QDate> &hashOfStringDate) {
    QStringList keys;
    stream >> keys;
    for (auto itKey = keys.begin();
         itKey != keys.end(); ++itKey) {
        stream >> hashOfStringDate[*itKey];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QHash<QString, QMap<QDate, int>> &hashOfStringDateInt)
{
    const QStringList &keys = hashOfStringDateInt.keys();
    stream << keys;
    for (auto itPair=hashOfStringDateInt.begin();
         itPair!=hashOfStringDateInt.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QHash<QString, QMap<QDate, int>> &hashOfStringDateInt)
{
    QStringList keys;
    stream >> keys;
    for (const auto &key : keys)
    {
        stream >> hashOfStringDateInt[key];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QHash<QString, QList<QDate>> &hashOfStringListDate)
{
    const QStringList &keys = hashOfStringListDate.keys();
    stream << keys;
    for (auto itPair=hashOfStringListDate.begin();
         itPair!=hashOfStringListDate.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QHash<QString, QList<QDate>> &hashOfStringListDate)
{
    QStringList keys;
    stream >> keys;
    for (const auto &key : keys)
    {
        stream >> hashOfStringListDate[key];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QHash<QString, QSet<QString>> &hashOfStringSet)
{
    const QStringList &keys = hashOfStringSet.keys();
    stream << keys;
    for (auto itPair=hashOfStringSet.begin();
         itPair!=hashOfStringSet.end(); ++itPair) {
        stream << itPair.value();
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QHash<QString, QSet<QString>> &hashOfStringSet)
{
    QStringList keys;
    stream >> keys;
    for (const auto &key : keys)
    {
        stream >> hashOfStringSet[key];
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QList<QMap<QDate, double>> &listOfMap_date_double)
{
    stream << int(listOfMap_date_double.size());
    for (const auto &hash : listOfMap_date_double)
    {
        stream << hash;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream, QList<QMap<QDate, double>> &listOfMap_date_double)
{
    int n = 0;
    stream >> n;
    for (int i = 0; i < n; ++i)
    {
        QMap<QDate, double> hash;
        stream >> hash;
        listOfMap_date_double << hash;
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QList<QHash<QString, QVariant>> &listOfHashOfStringVariant) {
    stream << int(listOfHashOfStringVariant.size());
    for (const auto &hash : listOfHashOfStringVariant)
    {
        stream << hash;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream,
                        QList<QHash<QString, QVariant>> &listOfHashOfStringVariant) {
    int n = 0;
    stream >> n;
    for (int i = 0; i < n; ++i)
    {
        QHash<QString, QVariant> hash;
        stream >> hash;
        listOfHashOfStringVariant << hash;
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QString, QString>> &hashOfHashString)
{
    const QStringList &keys1 = hashOfHashString.keys();
    stream << keys1;
    for (auto itLine=hashOfHashString.begin(); itLine!=hashOfHashString.end(); ++itLine) {
        const QStringList &keys2 = itLine->keys();
        stream << keys2;
        for (auto itElement = itLine.value().begin();
             itElement != itLine.value().end(); ++itElement) {
            stream << itElement.value();
        }
    }
    return stream;

}
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QString, QString>> &hashOfHashString)
{
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1 != keys1.end(); ++itKey1)
    {
        hashOfHashString[*itKey1] = QHash<QString, QString>();
        QStringList keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2)
        {
            QString value;
            stream >> value;
            hashOfHashString[*itKey1][*itKey2] = value;
        }
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QHash<QString, double>> &hashOfHashDouble)
{
    const QStringList &keys1 = hashOfHashDouble.keys();
    stream << keys1;
    for (auto itLine=hashOfHashDouble.begin(); itLine!=hashOfHashDouble.end(); ++itLine)
    {
        const QStringList &keys2 = itLine->keys();
        stream << keys2;
        for (auto itElement = itLine.value().begin();
             itElement != itLine.value().end(); ++itElement)
        {
            stream << itElement.value();
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QHash<QString, double>> &hashOfHashDouble)
{
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1 != keys1.end(); ++itKey1) {
        hashOfHashDouble[*itKey1] = QHash<QString, double>();
        QStringList keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2) {
            /*
            QString valueStr;
            stream >> valueStr;
            hashOfHashDouble[*itKey1][*itKey2] = valueStr.toDouble();
            //*/
            double value = 0.;
            stream >> value;
            if (qAbs(value) < 0.00001) {
                value = 0.;
            }
            hashOfHashDouble[*itKey1][*itKey2] = value;
        }
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (QDataStream &stream,
                        const QHash<QString, QHash<QString, QVariant>> &hashOfHashVariant)
{
    const QStringList &keys1 = hashOfHashVariant.keys();
    stream << keys1;
    for (auto itLine=hashOfHashVariant.begin(); itLine!=hashOfHashVariant.end(); ++itLine)
    {
        const QStringList &keys2 = itLine->keys();
        stream << keys2;
        for (auto itElement = itLine.value().begin();
             itElement != itLine.value().end(); ++itElement)
        {
            stream << itElement.value();
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (QDataStream &stream,
                        QHash<QString, QHash<QString, QVariant>> &hashOfHashVariant)
{
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1 != keys1.end(); ++itKey1)
    {
        hashOfHashVariant[*itKey1] = QHash<QString, QVariant>();
        QStringList keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2)
        {
            QVariant value;
            stream >> value;
            hashOfHashVariant[*itKey1][*itKey2] = value;
        }
    }
    return stream;
}
//----------------------------------------
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QMap<QDate, QHash<QString, double>>> &hashOfMapOfHashDouble)
{
    QStringList keys1 = hashOfMapOfHashDouble.keys();
    stream << keys1;
    for (auto it1 = hashOfMapOfHashDouble.begin(); it1!=hashOfMapOfHashDouble.end(); ++it1) {
        QList<QDate> keys2 = hashOfMapOfHashDouble[it1.key()].keys();
        stream << keys2;
        for (auto it2 = it1.value().begin(); it2 != it1.value().end(); ++it2) {
            stream << hashOfMapOfHashDouble[it1.key()][it2.key()];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QMap<QDate, QHash<QString, double>>> &hashOfMapOfHashDouble) {
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1->end(); ++itKey1) {
        QList<QDate> keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2) {
            stream >> hashOfMapOfHashDouble[*itKey1][*itKey2];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QHash<QString, QMap<QDate, QVariant>>> &hashOfHaghOfMapDateVariant) {
    QStringList keys1 = hashOfHaghOfMapDateVariant.keys();
    stream << keys1;
    for (auto it1 = hashOfHaghOfMapDateVariant.begin();
         it1!=hashOfHaghOfMapDateVariant.end(); ++it1) {
        auto keys2 = hashOfHaghOfMapDateVariant[it1.key()].keys();
        stream << keys2;
        for (auto it2 = it1.value().begin(); it2 != it1.value().end(); ++it2) {
            stream << hashOfHaghOfMapDateVariant[it1.key()][it2.key()];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QHash<QString, QMap<QDate, QVariant>>> &hashOfHaghOfMapDateVariant) {
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1->end(); ++itKey1) {
        QStringList keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2) {
            stream >> hashOfHaghOfMapDateVariant[*itKey1][*itKey2];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QList<bool> &listOfBools)
{
    stream << listOfBools.size();
    for (auto it = listOfBools.begin();
         it != listOfBools.end(); ++it) {
        stream << *it;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QList<bool> &listOfBools)
{
    int size = 0;
    stream >> size;
    for (int i=0; i<size; ++i) {
        bool value = true;
        stream >> value;
        listOfBools << value;
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QHash<QString, QSet<QString>>> &hashOfHaghOfVariantSet)
{
    QStringList keys1 = hashOfHaghOfVariantSet.keys();
    stream << keys1;
    for (auto it1 = hashOfHaghOfVariantSet.begin();
         it1!=hashOfHaghOfVariantSet.end(); ++it1) {
        auto keys2 = hashOfHaghOfVariantSet[it1.key()].keys();
        stream << keys2;
        for (auto it2 = it1.value().begin(); it2 != it1.value().end(); ++it2) {
            stream << hashOfHaghOfVariantSet[it1.key()][it2.key()];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QHash<QString, QSet<QString>>> &hashOfHaghOfVariantSet)
{
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1 != keys1.end(); ++itKey1) {
        QStringList keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2) {
            stream >> hashOfHaghOfVariantSet[*itKey1][*itKey2];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QHash<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant)
{
    QStringList keys1 = hashOfHashOfHashStringVariant.keys();
    stream << keys1;
    for (auto it1 = hashOfHashOfHashStringVariant.begin();
         it1!=hashOfHashOfHashStringVariant.end(); ++it1) {
        auto keys2 = hashOfHashOfHashStringVariant[it1.key()].keys();
        stream << keys2;
        for (auto it2 = it1.value().begin(); it2 != it1.value().end(); ++it2) {
            stream << hashOfHashOfHashStringVariant[it1.key()][it2.key()];
        }
    }
    return stream;

}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QHash<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant)
{
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1 != keys1.end(); ++itKey1) {
        QList<QDate> keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2) {
            stream >> hashOfHashOfHashStringVariant[*itKey1][*itKey2];
        }
    }
    return stream;
}
//----------------------------------------
QDataStream & operator << (
        QDataStream &stream,
        const QHash<QString, QMap<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant)
{
    QStringList keys1 = hashOfHashOfHashStringVariant.keys();
    stream << keys1;
    for (auto it1 = hashOfHashOfHashStringVariant.begin();
         it1!=hashOfHashOfHashStringVariant.end(); ++it1) {
        auto keys2 = hashOfHashOfHashStringVariant[it1.key()].keys();
        stream << keys2;
        for (auto it2 = it1.value().begin(); it2 != it1.value().end(); ++it2) {
            stream << hashOfHashOfHashStringVariant[it1.key()][it2.key()];
        }
    }
    return stream;

}
//----------------------------------------
QDataStream & operator >> (
        QDataStream &stream,
        QHash<QString, QMap<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant)
{
    QStringList keys1;
    stream >> keys1;
    for (auto itKey1 = keys1.begin(); itKey1 != keys1.end(); ++itKey1) {
        QList<QDate> keys2;
        stream >> keys2;
        for (auto itKey2 = keys2.begin(); itKey2 != keys2.end(); ++itKey2) {
            stream >> hashOfHashOfHashStringVariant[*itKey1][*itKey2];
        }
    }
    return stream;
}
//----------------------------------------
