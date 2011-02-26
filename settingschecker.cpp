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
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
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
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
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
