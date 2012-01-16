#include "updatedialog.h"
#include "ui_updatedialog.h"
#include <qdesktopservices.h>

UpdateDialog::UpdateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::setDownloadUrl(UpdateInfo *info)
{
    ui->labelVersion->setText(info->versionStr);
    ui->textEdit->setPlainText(info->changelog);
    m_url = info->downloadLink;
}

void UpdateDialog::on_commandLinkButton_clicked()
{
    QDesktopServices::openUrl(QUrl(m_url));
}
