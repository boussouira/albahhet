#include "settingschecker.h"
#include <qapplication.h>
#include <qstringlist.h>
#include <qfile.h>

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
            indexesList.removeAll(index);
            settings.remove(index);

            if(index == currentIndex) {
                if(!indexesList.isEmpty())
                    settings.setValue("current_index", indexesList.first());
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

    if(!QFile::exists(indexPath) || !QFile::exists(shaPath))
        return false;

    if(ramSize <= 0)
        settings.setValue("ram_size", 100);

    settings.endGroup();

    return true;
}
