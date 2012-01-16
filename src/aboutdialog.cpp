#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "common.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->labelVersion->setText(APP_VERSION_STR);
#ifdef GITCHANGENUMBER
    ui->labelRevision->setText(QString("git: %1 (rev: %2)").arg(GITVERSION).arg(GITCHANGENUMBER));
#else
    ui->labelRevision->hide();
#endif
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
