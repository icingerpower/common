#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#include "../web/WebPageGetter.h"

#include "ExceptionCurrencyRate.h"

#include "CurrencyRateManager.h"

//----------------------------------------------------------
CurrencyRateManager::CurrencyRateManager(QObject *parent)
: QObject(parent)
{
    QSettings settings;
    m_workingDir = QFileInfo(settings.fileName()).dir();
}
//----------------------------------------------------------
CurrencyRateManager *CurrencyRateManager::instance()
{
    static CurrencyRateManager manager;
    return &manager;
}
//----------------------------------------------------------
void CurrencyRateManager::setWorkingDir(const QString &path)
{
    m_workingDir = path;
}
//----------------------------------------------------------
double extractRateFromContent(const QString &content,
                               const QString &currency) {
    double rate = 1.;
    if (currency != "EUR") {
        if (!content.contains(currency)) {
            qDebug() << content;
        }
        QString rateString = content.split(
                    QString("\"%1\":").arg(currency))[1];
        if (rateString.contains(",")) {
            rate = rateString.split(",")[0].toDouble();
        } else {
            rate = rateString.split("}")[0].toDouble();
        }
    }
    return rate;
}
//----------------------------------------------------------
double CurrencyRateManager::retrieveCurrency(
        const QString &source,
        const QString &dest,
        const QDate &date) const
{
    Q_ASSERT(date < QDate::currentDate());
    double rate = 1.0;
    if(source != dest) {
        WebPageGetter pageGetter;
        //QString url = "http://api.fixer.io/";
        QString url = "http://data.fixer.io/api/";
        url += date.toString("yyyy-MM-dd");
        url += "?access_key=28191005a67fa45a0e00300fc582e67d";
        url += "&symbols=";
        bool hasEuro = source == "EUR" || dest == "EUR";
        if(!hasEuro) {
            url += source;
            url += ",";
            url += dest;
        } else {
            url += QString(source + dest).replace("EUR", "");
        }
        qDebug() << "url to retrieve currency:" << url;
        QString rateInfo
                = pageGetter.getPageContent(
                    url);
        if (rateInfo.isEmpty()) {
            ExceptionCurrencyRate exception;
            exception.setUrl(url);
            exception.raise();
        }
        //Q_ASSERT(!rateInfo.isEmpty());
        double rateSource
                = extractRateFromContent(rateInfo,
                                         source);
        double rateDest
                = extractRateFromContent(rateInfo,
                                         dest);
        rate = rateDest / rateSource;
    }
    return rate;
}
//----------------------------------------------------------
double CurrencyRateManager::rate(
        const QString &source,
        const QString &dest,
        const QDate &date) const
{
    Q_ASSERT(date.isValid());
    Q_ASSERT(source != dest);
    Q_ASSERT(source.size() == 3);
    Q_ASSERT(dest.size() == 3);
    QString key = date.toString("yyyy_MM_dd")
            + "/" + dest + "-" + source;
    QString keyReversed = date.toString("yyyy_MM_dd")
            + "/" + source + "-" + dest;
    auto settingsRates = _settingsRates();
    if (!settingsRates->contains(key)) {
        double rate = retrieveCurrency(source, dest, date);
        settingsRates->setValue(key, rate);
        settingsRates->setValue(keyReversed, 1/rate);
        /*
        // TODO put back original
        if (source == "GBP" && dest == "EUR") {
            return 1.2;
        } else if (source == "USD" && dest == "EUR") {
            return 0.9;
        } else if (source == "SEK" && dest == "EUR") {
            return 0.01;
        } else if (source == "PLN" && dest == "EUR") {
            return 0.2;
        } else if (source == "CAD" && dest == "EUR") {
            return 0.73;
        } else {
            Q_ASSERT(false);
        }
        //*/
        settingsRates->sync();
    }
    double rate = settingsRates->value(key, 1.).toDouble();
    return rate;
}
//----------------------------------------------------------
double CurrencyRateManager::convert(
        double amount,
        const QString &curSource,
        const QString &curDest,
        const QDate &date)
{
    if (curSource != curDest) {
        Q_ASSERT(curSource.length() == 3 & curDest.length() == 3
                 && curSource.toUpper() != curDest.toUpper());
        double rate = this->rate(curSource, curDest, date);
        return amount * rate;
    }
    return amount;
}
//----------------------------------------------------------
QSharedPointer<QSettings> CurrencyRateManager::_settingsRates() const
{
    QString filePath
            = m_workingDir.absoluteFilePath("currency-rates.txt");
    QSharedPointer<QSettings> settingsRates(
                new QSettings(
                    filePath, QSettings::IniFormat));
    return settingsRates;
}
//----------------------------------------------------------
