#ifndef COMMON_H
#define COMMON_H

#include <qwidget.h>
#include <CLucene/SharedHeader.h>
#include "indexinfo.h"
#include "booksdb.h"

#define APP_VERSION "1.0.1"
#define APP_NAME QObject::trUtf8("الباحث الشامل")

#define TCHAR_TO_QSTRING(s)     TCharToQString(s)
#define FIELD_TO_INT(name, d)   _wtoi(d->get(_T(name)))
#define QSTRING_TO_TCHAR(s)     (TCHAR*)s.utf16()

#define SETTINGS_FILE (qApp->applicationDirPath() + "/settings.ini")

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
void deleteBooksDb(BooksDB *db);

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


#endif // COMMON_H
