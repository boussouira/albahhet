#ifndef COMMON_H
#define COMMON_H

#include <qwidget.h>
#include <qdesktopservices.h>
#include <qdir.h>
#include <CLucene/SharedHeader.h>
#include "indexinfo.h"
#include "booksdb.h"

#define APP_VERSION 0x010009
#define APP_VERSION_STR "1.0.9"
#define APP_NAME QObject::trUtf8("الباحث الشامل")

#define TCHAR_TO_QSTRING(s)     TCharToQString(s)
#define QSTRING_TO_TCHAR(s)     (TCHAR*)s.utf16()

#define LOCAL_SETTINGS_FILE (qApp->applicationDirPath() + "/settings.ini")
#define LOG_FILE QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation)).filePath("session.txt")

enum PULRAL{
    SECOND  = 1,
    MINUTE  = 2,
    HOUR    = 3,
    BOOK    = 4
};

TCHAR* QStringToTChar(const QString &str);
QString TCharToQString(const TCHAR *string);

QString arPlural(int count, PULRAL word, bool html=false);

void normaliseSearchString(QString &text);

void hideHelpButton(QWidget *w);
void clearShorts(QString &str);
void forceRTL(QWidget *widget);

#define ADD_QTREEWIDGET_ITEM(name, value) { \
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget); \
        item->setData(0, Qt::DisplayRole, trUtf8(name)); \
        item->setData(1, Qt::DisplayRole, value); \
        itemList.append(item); \
        }

#define PROGRESS_DIALOG_STEP(text)     progress.setValue(progress.value()+1); \
                                progress.setLabelText(trUtf8("جاري " text "..."));

#define DB_OPEN_ERROR(path) qCritical("[%s:%d] Cannot open database at \"%s\".", \
                                __FILE__, \
                                __LINE__, \
                                qPrintable(path));

#define SQL_ERROR(error) qCritical("[%s:%d] SQL error: \"%s\".", \
                                          __FILE__, \
                                          __LINE__, \
                                          qPrintable(error));

#define DELETE_DB(p) { QStringList connList = p->connections(); \
                        delete p; \
                        foreach(QString conn, connList){QSqlDatabase::removeDatabase(conn);}}


#endif // COMMON_H
