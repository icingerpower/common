#ifndef TYPES_H
#define TYPES_H

#include <QStringList>
#include <QVariant>
#include <QMetaType>
#include <QSet>
#include <QHash>
#include <QDate>
#include <QSize>

inline void initTypes(){
    qRegisterMetaType<QList<QStringList>>();
}
#define INIT_TYPES qRegisterMetaType<QList<QStringList>>()
QDataStream & operator << (QDataStream &stream, const QList<QStringList> &listOfStringList);
QDataStream & operator >> (QDataStream &stream, QList<QStringList> &listOfStringList);

QDataStream & operator << (QDataStream &stream, const QList<QList<QVariant>> &listOfVariantList);
QDataStream & operator >> (QDataStream &stream, QList<QList<QVariant>> &listOfVariantList);

QDataStream & operator << (QDataStream &stream, const QList<QSize> &sizes);
QDataStream & operator >> (QDataStream &stream, QList<QSize> &sizes);

QDataStream & operator << (QDataStream &stream, const QList<QDate> &dates);
QDataStream & operator >> (QDataStream &stream, QList<QDate> &dates);

QDataStream & operator << (QDataStream &stream, const QMap<QDate, int> &mapDateInt);
QDataStream & operator >> (QDataStream &stream, QMap<QDate, int> &mapDateInt);

QDataStream & operator << (QDataStream &stream, const QSet<QString> &set);
QDataStream & operator >> (QDataStream &stream, QSet<QString> &set);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QString> &hashOfStrings);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QString> &hashOfStrings);

QDataStream & operator << (QDataStream &stream, const QHash<QString, bool> &hashOfStringBool);
QDataStream & operator >> (QDataStream &stream, QHash<QString, bool> &hashOfStringBool);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QVariant> &hashOfStringVariant);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QVariant> &hashOfStringVariant);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QDate> &hashOfStringDate);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QDate> &hashOfStringDate);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QMap<QDate, int>> &hashOfStringDateInt);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QMap<QDate, int>> &hashOfStringDateInt);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QList<QDate>> &hashOfStringListDate);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QList<QDate>> &hashOfStringListDate);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QSet<QString>> &hashOfStringSet);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QSet<QString>> &hashOfStringSet);

QDataStream & operator << (QDataStream &stream, const QList<QHash<QString, QVariant>> &listOfHashOfStringVariant);
QDataStream & operator >> (QDataStream &stream, QList<QHash<QString, QVariant>> &listOfHashOfStringVariant);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QString, QString>> &hashOfHashString);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QString, QString>> &hashOfHashString);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QString, double>> &hashOfHashDouble);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QString, double>> &hashOfHashDouble);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QString, QVariant>> &hashOfHashVariant);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QString, QVariant>> &hashOfHashVariant);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QMap<QDate, QHash<QString, double>>> &hashOfMapOfHashDouble);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QMap<QDate, QHash<QString, double>>> &hashOfMapOfHashDouble);

QDataStream & operator << (QDataStream &stream, const QList<bool> &listOfBools);
QDataStream & operator >> (QDataStream &stream, QList<bool> &listOfBools);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QString, QMap<QDate, QVariant>>> &hashOfHashOfMapDateVariant);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QString, QMap<QDate, QVariant>>> &hashOfHashOfMapDateVariant);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QString, QSet<QString>>> &hashOfHashOfVariantSet);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QString, QSet<QString>>> &hashOfHashOfVariantSet);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QHash<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QHash<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant);

QDataStream & operator << (QDataStream &stream, const QHash<QString, QMap<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant);
QDataStream & operator >> (QDataStream &stream, QHash<QString, QMap<QDate, QHash<QString, QVariant>>> &hashOfHashOfHashStringVariant);

#define REGISTER_QT_METATYPES_SIMPLE \
    qRegisterMetaType<QList<QStringList>>(); \
    qRegisterMetaType<QList<QList<QVariant>>>(); \
    qRegisterMetaType<QSet<QString>>(); \
    qRegisterMetaType<QHash<QString, QString>>(); \
    qRegisterMetaType<QHash<QString, QVariant>>(); \
    qRegisterMetaType<QHash<QString, bool>>(); \
    qRegisterMetaType<QMap<QString, int>>(); \
    qRegisterMetaType<QList<bool>>();

#endif // TYPES_H
