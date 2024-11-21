#ifndef DIALOGEDITCUSTOMER_H
#define DIALOGEDITCUSTOMER_H

#include <QDialog>
#include <QSharedPointer>

#include "Customer.h"

namespace Ui {
class DialogEditCustomer;
}

class DialogEditCustomer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditCustomer(QWidget *parent = nullptr);
    ~DialogEditCustomer();
    QSharedPointer<Customer> getCustomer() const;
    bool wasAccepted() const;

public slots:
    void accept() override;
    void reject() override;

private:
    Ui::DialogEditCustomer *ui;
    bool m_accepted;
};

#endif // DIALOGEDITCUSTOMER_H
