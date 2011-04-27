#include "settingschecker.h"
#include "indexesmanager.h"
#include "settingsdialog.h"
#include <qapplication.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qdebug.h>

SettingsChecker::SettingsChecker()
{
}

void SettingsChecker::checkIndexes()
{
    IndexesManager manager;
    foreach (IndexInfo *index, manager.list()) {
        if(!checkIndex(index)) {
            int rep = QMessageBox::question(0,
                                            QObject::trUtf8("فحص الفهارس"),
                                            QObject::trUtf8("لم يتم العثور على بعض مجلدات او ملفات الفهرس %1"
                                                            "\n" "هل تريد حذفه؟").arg(index->name()),
                                            QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if(rep == QMessageBox::Yes) {
                qWarning() << "SettingsChecker:" << "Remove" << index->name() << "id:" << index->id();
                manager.remove(index);
            }
        }
    }
}

bool SettingsChecker::checkIndex(IndexInfo *index)
{
    QString indexPath = index->path();
    QString shaPath = index->shamelaPath();

    QDir indexDir(indexPath);
    if(!indexDir.exists()) {
        qWarning() << "Directory" << indexPath << "not found";
        return false;
    }

    QDir shaDir(shaPath);
    if(!shaDir.exists()) {
        qWarning() << "Directory" << shaPath << "not found";
        return false;
    }

    return true;
}

void SettingsChecker::update()
{
    QSettings settings;

    int currentVersion = settings.value("currentVersion", 0).toInt();

    if(currentVersion == 0) {
        if(QFile::exists(LOCAL_SETTINGS_FILE)) {
            qDebug("Try to upgard...");

            QSettings oldSettings(LOCAL_SETTINGS_FILE, QSettings::IniFormat);
            QStringList allKeys;
            allKeys = oldSettings.allKeys();

            foreach(QString key, allKeys) {
                settings.setValue(key, oldSettings.value(key));
            }

            if(!QFile::remove(LOCAL_SETTINGS_FILE))
                qWarning("Can't delete old settings file: \"%s\"", qPrintable(LOCAL_SETTINGS_FILE));
            else
                qDebug("Delete old settings file...");
        }
    }

    settings.setValue("currentVersion", APP_VERSION);
}

void SettingsChecker::updateToXml()
{
    QSettings settings;

    if(!settings.value("usingXml", false).toBool()) {
        qDebug("Try to upgard to xml...");

        IndexesManager manager;
        foreach(QString key, settings.childGroups()) {
            if(key.startsWith("i_")) {
                manager.add(settings, key);
                settings.remove(key);
            }
        }

        settings.remove("indexes_list");
        settings.setValue("usingXml", true);
    }
}

void SettingsChecker::indexingConfig()
{
    QSettings settings;

    if(!settings.value("haveIndexingConfig", false).toBool()) {

        SettingsDialog dialog;
        dialog.setCurrentPage(1);

        QMessageBox::information(0,
                                 QObject::trUtf8("اعدادات الفهرسة"),
                                 QObject::trUtf8("من فضلك قم بضبط اعدادات الفهرسة"));
        if(dialog.exec() == SettingsDialog::Accepted)
            settings.setValue("haveIndexingConfig", true);

    }
}
