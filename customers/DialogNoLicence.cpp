#include <QClipboard>
#include <QApplication>

#include "CustomerTableModel.h"

#include "DialogNoLicence.h"
#include "ui_DialogNoLicence.h"

DialogNoLicence::DialogNoLicence(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNoLicence)
{
    ui->setupUi(this);
    ui->lineEdit->setText(
        CustomerTableModel::ethernetAddress());
    connect(ui->buttonCopy,
            &QPushButton::clicked,
            this,
            &DialogNoLicence::copyMessage);
}

DialogNoLicence::~DialogNoLicence()
{
    delete ui;
}

void DialogNoLicence::copyMessage()
{
    auto clipboard = QApplication::clipboard();
    QString message("Hello,\n"
                    "This is my code for access: ");
    message += ui->lineEdit->text();
    clipboard->setText(message);
}

