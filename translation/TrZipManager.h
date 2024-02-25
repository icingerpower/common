#ifndef TRMANAGER_H
#define TRMANAGER_H

#include "TrTextManager.h"

//*
class TrZipManager : public TrTextManager
{
    Q_OBJECT

public:
    static TrZipManager *instance();
    static TrZipManager *createInstanceForUnitTest();
    virtual void displayTextTranslated(
            const QString &contained,
            const QString &langCode,
            Qt::CaseSensitivity cs = Qt::CaseInsensitive) override;
    int countHowMuchTranslatedAlready() const override;
    void convertFromText() override;
    void save() override;
    void saveUpdated() override;
    void loadTranslatedAlready() override;
    void loadInBufferLangTo(const QString &langTo) override;

protected:
    void readAllLines(
            QStringList &lines,
            const QString &langCode) const override;
    void writeAllLines(
            const QStringList &lines,
            const QString &langCode) const override;

private:
    explicit TrZipManager(QObject *parent = nullptr);
    QString _fileNameZip(const QString &langCode) const;
};


//*/
#endif // TRMANAGER_H
