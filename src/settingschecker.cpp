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
                                            QObject::tr("فحص الفهارس"),
                                            QObject::tr("لم يتم العثور على بعض مجلدات او ملفات الفهرس %1"
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
    QString indexPath = index->indexPath();
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

void SettingsChecker::indexingConfig()
{
    QSettings settings;

    if(!settings.value("haveIndexingConfig", false).toBool()) {

        SettingsDialog dialog;
        dialog.setCurrentPage(1);

        QMessageBox::information(0,
                                 QObject::tr("اعدادات الفهرسة"),
                                 QObject::tr("من فضلك قم بضبط اعدادات الفهرسة"));
        if(dialog.exec() == SettingsDialog::Accepted)
            settings.setValue("haveIndexingConfig", true);

    }
}
