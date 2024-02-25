#include <QDebug>

#include "MeasurementManager.h"

//----------------------------------------
MeasurementManager *MeasurementManager::instance()
{
    static MeasurementManager instance;
    return &instance;

}
//----------------------------------------
MeasurementManager::MeasurementManager()
{
}
//----------------------------------------
void MeasurementManager::displayN(const QString &message, int N)
{
    qInfo() << "MEASURE:" << message << N;
}
//----------------------------------------
void MeasurementManager::measure(const QString &message)
{
    QPair<QString, QDateTime> measure;
    measure.first = message;
    measure.second = QDateTime::currentDateTime();
    m_listOfMessageTime << measure;
    //qInfo() << "MEASURE:" << message << measure.second.toString("yyyy-MM-dd hh:mm:ss:zzz");
}
//----------------------------------------
void MeasurementManager::endMeasure()
{
    auto measure = m_listOfMessageTime.takeLast();
    auto currentDateTime = QDateTime::currentDateTime();
    qint64 ms = measure.second.msecsTo(currentDateTime);
    qint64 seconds = ms / 1000;
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;
    QString displayTime;
    if (hours > 0) {
        displayTime
                = QString::number(hours)
                + ":" + QString::number(minutes % 60)
                + ":" + QString::number(seconds % 60) + " sec";
    } else if (minutes > 0) {
        displayTime = QString::number(minutes)
                + ":" + QString::number(seconds % 60) + " sec";
    } else if (seconds > 10) {
        displayTime = QString::number(seconds) + " sec";
    } else {
        displayTime = QString::number(ms) + " ms";
    }
    if (ms > 10000) {
        ms /= 1000;
    }
    QTime time(0, 0, 0);
    time = time.addMSecs(ms);
    qInfo() << "MEASURE DONE:"
            << measure.first
            << currentDateTime.toString("yyyy-MM-dd hh:mm:ss:zzz")
            << displayTime
            << Qt::endl;
}
//----------------------------------------
