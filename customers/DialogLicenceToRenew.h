#ifndef DIALOGLICENCETORENEW_H
#define DIALOGLICENCETORENEW_H

#include <QDialog>

namespace Ui {
class DialogLicenceToRenew;
}

class DialogLicenceToRenew : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLicenceToRenew(QWidget *parent = nullptr);
    ~DialogLicenceToRenew();

private:
    Ui::DialogLicenceToRenew *ui;
};

#endif // DIALOGLICENCETORENEW_H
