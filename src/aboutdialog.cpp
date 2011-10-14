#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->labelVersion->setText(APP_VERSION_STR);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
