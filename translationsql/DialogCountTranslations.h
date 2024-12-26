#ifndef DIALOGCOUNTTRANSLATIONS_H
#define DIALOGCOUNTTRANSLATIONS_H

#include <QDialog>

class TrSqlManager;

namespace Ui {
class DialogCountTranslations;
}

class DialogCountTranslations : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCountTranslations(
            TrSqlManager *trSqlManager,
            const QStringList &langCodesFrom,
            const QStringList &langCodesTo,
            QWidget *parent = nullptr);
    ~DialogCountTranslations();

private:
    Ui::DialogCountTranslations *ui;
};

#endif // DIALOGCOUNTTRANSLATIONS_H
