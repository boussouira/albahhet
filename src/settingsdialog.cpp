#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "common.h"
#include <qsettings.h>
#include <qfont.h>
#include <QWebSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    hideHelpButton(this);

    QSettings settings;

    ui->spinResultPeerPage->setValue(settings.value("resultPeerPage", 10).toInt());
    ui->checkOpenNewTab->setChecked(settings.value("useTabs", true).toBool());
    ui->checkScanIndexes->setChecked(settings.value("checkIndexes", true).toBool());
    ui->checkShowNewIndex->setChecked(settings.value("showNewIndexMsg", true).toBool());
    ui->checkOptimizeIndex->setChecked(settings.value("optimizeIndex", false).toBool());
    ui->spinThreadCount->setValue(settings.value("threadCount", QThread::idealThreadCount()).toInt());

    settings.beginGroup("BooksViewer");
    ui->checkHLFirstPage->setChecked(settings.value("highlightOnlyFirst", true).toBool());
    QString fontString = settings.value("fontFamily", QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont)).toString();
    int fontSize = settings.value("fontSize", QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize)).toInt();
    settings.endGroup();

    QFont font;
    font.fromString(fontString);

    if(fontSize < 9 || 72 < fontSize)
        fontSize = 9;

    ui->fontComboBox->setCurrentFont(font);
    ui->comboFontSize->setCurrentIndex(ui->comboFontSize->findText(QString::number(fontSize)));

    setRamSizes();

    int currentSize = settings.value("ramSize", 100).toInt();
    for(int i=0; i<ui->comboBox->count(); i++) {
        if(ui->comboBox->itemData(i).toInt() == currentSize) {
            ui->comboBox->setCurrentIndex(i);
            break;
        }
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::saveSettings()
{
    QSettings settings;

    settings.setValue("resultPeerPage", ui->spinResultPeerPage->value());
    settings.setValue("useTabs", ui->checkOpenNewTab->isChecked());
    settings.setValue("checkIndexes", ui->checkScanIndexes->isChecked());
    settings.setValue("showNewIndexMsg", ui->checkShowNewIndex->isChecked());
    settings.setValue("threadCount", ui->spinThreadCount->value());
    settings.setValue("optimizeIndex", ui->checkOptimizeIndex->isChecked());
    settings.setValue("ramSize", ui->comboBox->itemData(ui->comboBox->currentIndex()));

    settings.beginGroup("BooksViewer");
    settings.setValue("fontFamily", ui->fontComboBox->currentFont().toString());
    settings.setValue("fontSize", ui->comboFontSize->currentText());
    settings.setValue("highlightOnlyFirst", ui->checkHLFirstPage->isChecked());
    settings.endGroup();

    QWebSettings::globalSettings()->setFontFamily(QWebSettings::StandardFont, ui->fontComboBox->currentFont().toString());
    QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFontSize, ui->comboFontSize->currentText().toInt());
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

void SettingsDialog::setRamSizes()
{
    ui->comboBox->addItem(trUtf8("%1 ميغا").arg(100), 100);
    ui->comboBox->addItem(trUtf8("%1 ميغا").arg(200), 200);
    ui->comboBox->addItem(trUtf8("%1 ميغا").arg(300), 300);
    ui->comboBox->addItem(trUtf8("%1 ميغا").arg(500), 500);
    ui->comboBox->addItem(trUtf8("%1 جيغا").arg(1), 1000);
    ui->comboBox->addItem(trUtf8("%1 جيغا").arg(1.5), 1500);
    ui->comboBox->addItem(trUtf8("%1 جيغا").arg(2), 2000);
    ui->comboBox->addItem(trUtf8("%1 جيغا").arg(3), 3000);
}

void SettingsDialog::setCurrentPage(int index)
{
    ui->tabWidget->setCurrentIndex(index);
}
