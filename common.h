#ifndef COMMON_H
#define COMMON_H

#include <QtSql>
#include <QDebug>
#include <QStringListModel>
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

#ifndef USE_MIL_SEC
        #define miTOsec(x) (x/1000.0)
        #define SECONDE_AR "ËÇäíÉ"
#else
        #define miTOsec(x) x
        #define SECONDE_AR "ÌÒÁ ãä ÇáËÇäíÉ"
#endif

#define _toBInt(x) ((x-(int)x) > 0) ? ((int)x)+1 : (int)x
#define _atLeastOne(x) (x > 0 ? x : 1)

#define INDEX_PATH  "book_index"

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::search;

#endif // COMMON_H
