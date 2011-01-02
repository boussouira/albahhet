#ifndef COMMON_H
#define COMMON_H

#include <QMessageBox>
#include <CLucene.h>
#include <CLucene/StdHeader.h>

#include <CLucene/_clucene-config.h>
#include <CLucene/config/repl_tchar.h>
#include <CLucene/config/repl_wchar.h>
#include <CLucene/util/CLStreams.h>
#include <CLucene/util/Misc.h>
#include <CLucene/util/StringBuffer.h>
#include <CLucene/util/dirent.h>

#include <CLucene/search/IndexSearcher.h>
//test for memory leaks:
#ifdef _MSC_VER
#ifdef _DEBUG
        #define _CRTDBG_MAP_ALLOC
        #include <stdlib.h>
        #include <crtdbg.h>
#endif
#endif

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cctype>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <cmath>

#ifdef Q_OS_WIN32
        #define TCHAR_TO_QSTRING(s)   QString::fromUtf16((const ushort*) s)
        #define FIELD_TO_INT(name, d) QString::fromUtf16((const ushort*)d->get(_T(name))).toInt()
        #define QSTRING_TO_TCHAR(s) (const wchar_t*) s.utf16()
        #define WIN32_LEAN_AND_MEAN
#else
        #define TCHAR_TO_QSTRING(s)   QString::fromWCharArray(s)
        #define FIELD_TO_INT(name, d) QString::fromWCharArray(d->get(_T(name))).toInt()
        #define QSTRING_TO_TCHAR(s) s.toStdWString().c_str()
        #include "mdbconverter.h"
#endif

//#define _ceil(x) ((x-(int)x) > 0) ? ((int)x)+1 : (int)x
#define _ceil(x) ceil(x)

#define SETTINGS_FILE qApp->applicationDirPath()+"/settings.ini"
#define NEW_QSETTINGS QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

#define ADD_QTREEWIDGET_ITEM(x, y) {QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget); \
        item->setData(0, Qt::DisplayRole, trUtf8(x)); \
        item->setData(1, Qt::DisplayRole, y); \
        itemList.append(item);}

#define FORCE_RTL(x)    x->setLayoutDirection(Qt::LeftToRight); \
                        x->setLayoutDirection(Qt::RightToLeft);

#define NORMALISE_SEARCH_STRING(x)  x.replace(QRegExp(trUtf8("ـفق")), "("); \
                                    x.replace(QRegExp(trUtf8("ـغق")), ")"); \
                                    x.replace(QRegExp(trUtf8("ـ[أا]و")), "OR"); \
                                    x.replace(QRegExp(trUtf8("ـو")), "AND"); \
                                    x.replace(QRegExp(trUtf8("ـبدون")), "NOT"); \
                                    x.replace(trUtf8("؟"), "?");

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::search;

#endif // COMMON_H
