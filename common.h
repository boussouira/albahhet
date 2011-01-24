#ifndef COMMON_H
#define COMMON_H

#include <qobject.h>
#include <cmath>
#include "indexinfo.h"

#define APP_VERSION "0.8"
#define APP_NAME QObject::trUtf8("الباحث الشامل")

#ifdef Q_OS_WIN32
        #define TCHAR_TO_QSTRING(s)   QString::fromUtf16((const ushort*) s)
        #define FIELD_TO_INT(name, d) _wtoi(d->get(_T(name)))
//        #define FIELD_TO_INT(name, d) QString::fromUtf16((const ushort*)d->get(_T(name))).toInt()
        #define QSTRING_TO_TCHAR(s) (const wchar_t*) s.utf16()
#else
        #define TCHAR_TO_QSTRING(s)   QString::fromWCharArray(s)
        #define FIELD_TO_INT(name, d) QString::fromWCharArray(d->get(_T(name))).toInt()
        #define QSTRING_TO_TCHAR(s) s.toStdWString().c_str()
        #include "mdbconverter.h"
#endif

#define _ceil(x) ceil(x)

#define SETTINGS_FILE (qApp->applicationDirPath() + "/settings.ini")
#define NEW_QSETTINGS QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

#define INDEX_HASH(name) QString("i_%1").arg(IndexInfo::nameHash(name))

#define DEL_DB(x) QSqlDatabase::removeDatabase(x);

#define ADD_QTREEWIDGET_ITEM(name, value) { \
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget); \
        item->setData(0, Qt::DisplayRole, trUtf8(name)); \
        item->setData(1, Qt::DisplayRole, value); \
        itemList.append(item); \
        }

#define FORCE_RTL(x)    x->setLayoutDirection(Qt::LeftToRight); \
                        x->setLayoutDirection(Qt::RightToLeft);

#define NORMALISE_SEARCH_STRING(x)  x.replace(QRegExp(trUtf8("ـفق")), "("); \
                                    x.replace(QRegExp(trUtf8("ـغق")), ")"); \
                                    x.replace(QRegExp(trUtf8("ـ[أا]و")), "OR"); \
                                    x.replace(QRegExp(trUtf8("ـو")), "AND"); \
                                    x.replace(QRegExp(trUtf8("ـبدون")), "NOT"); \
                                    x.replace(trUtf8("؟"), "?");

#endif // COMMON_H
