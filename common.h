#ifndef COMMON_H
#define COMMON_H

#include <qobject.h>
#include <CLucene/clucene-config.h>
#include <CLucene/SharedHeader.h>
#include "indexinfo.h"
#include "booksdb.h"

#define APP_VERSION "1.0"
#define APP_NAME QObject::trUtf8("الباحث الشامل")

enum PULRAL{
    SECOND  = 1,
    MINUTE  = 2,
    HOUR    = 3,
    BOOK    = 4
};

TCHAR* QStringToTChar(const QString &str);
QString TCharToQString(const TCHAR *string);

QString arPlural(int count, PULRAL word, bool html=false);

QString indexHashName(QString name);
QString indexHashName(IndexInfo *index);

void normaliseSearchString(QString &text);
void deleteBooksDb(BooksDB *db);

#ifdef Q_OS_WIN32
        #define TCHAR_TO_QSTRING(s)     TCharToQString(s)
        #define FIELD_TO_INT(name, d)   _wtoi(d->get(_T(name)))
        #define QSTRING_TO_TCHAR(s)     (TCHAR*)s.utf16()
#else
        #define TCHAR_TO_QSTRING(s)   QString::fromWCharArray(s)
        #define FIELD_TO_INT(name, d) QString::fromWCharArray(d->get(_T(name))).toInt()
        #define QSTRING_TO_TCHAR(s) s.toStdWString().c_str()
        #include "mdbconverter.h"
#endif

#define SETTINGS_FILE (qApp->applicationDirPath() + "/settings.ini")

#define ADD_QTREEWIDGET_ITEM(name, value) { \
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget); \
        item->setData(0, Qt::DisplayRole, trUtf8(name)); \
        item->setData(1, Qt::DisplayRole, value); \
        itemList.append(item); \
        }

#define PROGRESS_DIALOG_STEP(text)     progress.setValue(progress.value()+1); \
                                progress.setLabelText(trUtf8("جاري " text "..."));

#define FORCE_RTL(x)    x->setLayoutDirection(Qt::LeftToRight); \
                        x->setLayoutDirection(Qt::RightToLeft);

#endif // COMMON_H
