#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include "updatechecker.h"

namespace Ui {
class UpdateDialog;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    UpdateDialog(QWidget *parent = 0);
    ~UpdateDialog();

    void setDownloadUrl(UpdateInfo *info);

private slots:
    void on_commandLinkButton_clicked();

private:
    QString m_url;
    Ui::UpdateDialog *ui;
};

#endif // UPDATEDIALOG_H
