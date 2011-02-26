#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "common.h"
#include <qsettings.h>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    hideHelpButton(this);

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    ui->spinBox->setValue(settings.value("resultPeerPage", 10).toInt());
    ui->checkBox->setChecked(settings.value("useTabs", true).toBool());
    ui->checkScanIndexes->setChecked(settings.value("checkIndexes", true).toBool());
    ui->checkShowNewIndex->setChecked(settings.value("showNewIndexMsg", true).toBool());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::saveSettings()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.setValue("resultPeerPage", ui->spinBox->value());
    settings.setValue("useTabs", ui->checkBox->isChecked());
    settings.setValue("checkIndexes", ui->checkScanIndexes->isChecked());
    settings.setValue("showNewIndexMsg", ui->checkShowNewIndex->isChecked());

    emit settingsUpdated();
}

void SettingsDialog::on_pushSave_clicked()
{
    saveSettings();
    accept();
}

void SettingsDialog::on_pushCancel_clicked()
{
    reject();
}
