#include "settingschecker.h"
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
    QSettings settings;
    QStringList indexesList =  settings.value("indexes_list").toStringList();
    QString currentIndex =  settings.value("current_index").toString();

    foreach(QString index, indexesList) {
        if(!checkIndex(index)) {
            int rep = QMessageBox::question(0,
                                            QObject::trUtf8("فحص الفهارس"),
                                            QObject::trUtf8("لم يتم العثور على بعض مجلدات او ملفات الفهرس %1"
                                                            "\n"
                                                            "هل تريد حذفه؟").arg(index)
                                            , QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
            if(rep == QMessageBox::Yes) {
                qWarning() << "SettingsChecker:" << "Remove" << index;
                indexesList.removeAll(index);
                settings.remove(index);

                if(index == currentIndex) {
                    if(!indexesList.isEmpty())
                        settings.setValue("current_index", indexesList.first());
                }
            }
        }
    }

    if(indexesList.isEmpty()) {
        settings.remove("indexes_list");
        settings.remove("current_index");
    } else {
        settings.setValue("indexes_list", indexesList);
    }

}

bool SettingsChecker::checkIndex(QString index)
{
    QSettings settings;
    settings.beginGroup(index);
    QString indexPath = settings.value("index_path").toString();
    QString shaPath = settings.value("shamela_path").toString();
    int ramSize = settings.value("ram_size").toInt();

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

    if(ramSize <= 0)
        settings.setValue("ram_size", 100);

    settings.endGroup();

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
