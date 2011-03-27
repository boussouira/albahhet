#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    void saveSettings();

signals:
    void settingsUpdated();

protected:
    Ui::SettingsDialog *ui;

protected slots:
    void on_pushSave_clicked();
    void on_pushCancel_clicked();
};

#endif // SETTINGSDIALOG_H
