#ifndef DIALOGNOLICENCE_H
#define DIALOGNOLICENCE_H

#include <QDialog>

namespace Ui {
class DialogNoLicence;
}

class DialogNoLicence : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNoLicence(QWidget *parent = nullptr);
    ~DialogNoLicence();

public slots:
    void copyMessage();

private:
    Ui::DialogNoLicence *ui;
};

#endif // DIALOGNOLICENCE_H
