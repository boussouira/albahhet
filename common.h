#ifndef COMMON_H
#define COMMON_H

#include <qobject.h>
#include <CLucene/clucene-config.h>
#include <CLucene/SharedHeader.h>
#include "indexinfo.h"

#define APP_VERSION "0.8"
#define APP_NAME QObject::trUtf8("الباحث الشامل")

#ifdef Q_OS_WIN32
        #define TCHAR_TO_QSTRING(s)     TCharToQString(s)
        #define FIELD_TO_INT(name, d)   _wtoi(d->get(_T(name)))
        #define QSTRING_TO_TCHAR(s)     QStringToTChar(s)
#else
        #define TCHAR_TO_QSTRING(s)   QString::fromWCharArray(s)
        #define FIELD_TO_INT(name, d) QString::fromWCharArray(d->get(_T(name))).toInt()
        #define QSTRING_TO_TCHAR(s) s.toStdWString().c_str()
        #include "mdbconverter.h"
#endif

TCHAR* QStringToTChar(const QString &str);
QString TCharToQString(const TCHAR *string);

enum PULRAL{
    SECOND  = 1,
    MINUTE  = 2,
    HOUR    = 3,
    BOOK    = 4
};

QString arPlural(int count, PULRAL word, bool html=false);

#define SETTINGS_FILE (qApp->applicationDirPath() + "/settings.ini")
#define NEW_QSETTINGS QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

#define INDEX_HASH(name) QString("i_%1").arg(IndexInfo::nameHash(name))

#define DEL_DB(name) QSqlDatabase::removeDatabase(name);
#define DEL_DBS(list) {foreach(QString n, list){ DEL_DB(n);}}
#define DEL_BOOKS_DB(db) {QStringList conn = db->connections(); delete db; DEL_DBS(conn);}

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

#define NORMALISE_SEARCH_STRING(x)  x.replace(QRegExp(trUtf8("ـفق")), "("); \
                                    x.replace(QRegExp(trUtf8("ـغق")), ")"); \
                                    x.replace(QRegExp(trUtf8("ـ[أا]و")), "OR"); \
                                    x.replace(QRegExp(trUtf8("ـو")), "AND"); \
                                    x.replace(QRegExp(trUtf8("ـبدون")), "NOT"); \
                                    x.replace(trUtf8("؟"), "?");

#endif // COMMON_H
