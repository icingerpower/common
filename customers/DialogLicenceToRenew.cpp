#include "DialogLicenceToRenew.h"
#include "ui_DialogLicenceToRenew.h"

DialogLicenceToRenew::DialogLicenceToRenew(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogLicenceToRenew)
{
    ui->setupUi(this);
}

DialogLicenceToRenew::~DialogLicenceToRenew()
{
    delete ui;
}
