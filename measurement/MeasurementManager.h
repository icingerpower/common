#ifndef MEASUREMENTMANAGER_H
#define MEASUREMENTMANAGER_H

#include <QString>
#include <QDateTime>

class MeasurementManager
{
public:
    static MeasurementManager *instance();
    MeasurementManager();
    void displayN(const QString &message, int N);
    void measure(const QString &message);
    void endMeasure();


private:
    QList<QPair<QString, QDateTime>> m_listOfMessageTime;
};

#endif // MEASUREMENTMANAGER_H
