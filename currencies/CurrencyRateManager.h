#ifndef CURRENCYRATEMANAGER_H
#define CURRENCYRATEMANAGER_H

#include <QtCore/qobject.h>
#include <QtCore/qdir.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qsettings.h>
#include <QtCore/qdatetime.h>

class CurrencyRateManager : public QObject
{
    Q_OBJECT
public:
    static CurrencyRateManager * instance();
    void setWorkingDir(const QString &path);
    double retrieveCurrency(const QString &source,
                          const QString &dest,
                          const QDate &date) const;
    double rate(const QString &source,
                const QString &dest,
                const QDate &date) const;
    double convert(double amount,
                   const QString &curSource,
                   const QString &curDest, const QDate &date);



private:
    explicit CurrencyRateManager(QObject *parent = nullptr);
    QSharedPointer<QSettings> _settingsRates() const;
    QDir m_workingDir;
};

#endif // CURRENCYRATEMANAGER_H
