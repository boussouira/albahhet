#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef Q_OS_WIN
	#define TCHAR_TO_QSTRING(s)   QString::fromUtf16((const ushort*) s)
	#define FIELD_TO_INT(name, d) QString::fromUtf16((const ushort*)d->get(_T(name))).toInt()
	#define QSTRING_TO_TCHAR(s) (const wchar_t*) s.utf16()
	#define WIN32_LEAN_AND_MEAN
#else
	#define TCHAR_TO_QSTRING(s)   QString::fromWCharArray(s)
	#define FIELD_TO_INT(name, d) QString::fromWCharArray(d->get(_T(name))).toInt()
	#define QSTRING_TO_TCHAR(s) s.toStdWString().c_str()
#endif

#ifndef USE_MIL_SEC
	#define miTOsec(x) (x/1000.0)
	#define SECONDE_AR "ثانية"
#else
	#define miTOsec(x) x
	#define SECONDE_AR "جزء من الثانية"
#endif

#include "arabicanalyzer.h"
#include <QMainWindow>
#include <QtSql>
#include <QDebug>
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBrowser>
#include <QStandardItemModel>

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

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::search;

struct result{
    QList<int> results;
    QList<float_t> scoring;
    int offest;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    QString hiText(const QString &text, const QString &strToHi);
    QStringList buildRegExp(const QString &str);

public slots:
    void startIndexing();
    void startSearching();
    void showStatistic();
    void indexDocs(IndexWriter* writer);
    Document* FileDocument(const QString &id, const QString &ayaText);
    QString cleanString(QString str);
    void resultsCount();
    void displayResults(/*result &pResult*/);

protected:
    QSqlDatabase m_quranDB;
    QSqlQuery *m_quranQuery;
    QString m_quranDBPath;
    QSqlQueryModel *m_resultModel;
    Ui::MainWindow *ui;
    QMap<QString, QChar> letterMap;
    int resultCount;
    result m_resultStruct;
    QStandardItemModel *resultModel;

private slots:
    void on_tableView_doubleClicked(QModelIndex index);
    void on_pushGoNext_clicked();
};

#endif // MAINWINDOW_H
