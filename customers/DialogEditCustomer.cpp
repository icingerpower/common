#include <QMessageBox>

#include "DialogEditCustomer.h"
#include "ui_DialogEditCustomer.h"

DialogEditCustomer::DialogEditCustomer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogEditCustomer)
{
    ui->setupUi(this);
    ui->dateEditPayment->setDate(QDate::currentDate().addYears(1));
    m_accepted = false;
}

DialogEditCustomer::~DialogEditCustomer()
{
    delete ui;
}

QSharedPointer<Customer> DialogEditCustomer::getCustomer() const
{
    QSharedPointer<Customer> customer(
                new Customer{
                    ui->lineEditEmail->text() + QDateTime::currentDateTime().toString("yyyy-MM-dd__hh-mm-ss__zzz")
                    , ui->lineEditEmail->text()
                    , ui->lineEditName->text()
                    , ui->dateEditPayment->date()
                    , ui->spinBoxNumberIps->value()
                });
    return customer;
}

bool DialogEditCustomer::wasAccepted() const
{
    return m_accepted;
}

void DialogEditCustomer::accept()
{
    static QRegularExpression regex(R"((^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$))");
    const QString &email = ui->lineEditEmail->text();
    if (email.isEmpty() || !regex.match(email).hasMatch())
    {
        QMessageBox::information(this,
                                 tr("Wrong email"),
                                 tr("You need to enter a valid email."));
    }
    else if (ui->lineEditName->text().trimmed().isEmpty())
    {
        QMessageBox::information(this,
                                 tr("No name"),
                                 tr("You need to enter a customer name."));
    }
    else if (ui->dateEditPayment->date() >= QDate::currentDate())
    {
        QMessageBox::information(this,
                                 tr("Wrong date"),
                                 tr("The date can’t be in the future."));
    }
    else
    {
        m_accepted = true;
        QDialog::accept();
    }
}

void DialogEditCustomer::reject()
{
    m_accepted = false;
    QDialog::reject();
}
