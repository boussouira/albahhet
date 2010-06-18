#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "arabicanalyzer.h"
#include "indexingdialg.h"
#include "indexthread.h"
#include <QMainWindow>
#include <QtSql>
#include <QDebug>
#include <QStringListModel>
#include <QMessageBox>
#include <QTextBrowser>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QSettings>

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
	#define SECONDE_AR "ثانية"
#else
	#define miTOsec(x) x
	#define SECONDE_AR "جزء من الثانية"
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

struct result{
    QList<int> ids;
    QList<int> bookid;
    QList<float_t> scoring;
    int page;
    int pageCount;
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
    QString abbreviate(QString str, int size);
    QString getIndexSize();
    QString getBookSize();
    void writeLog(int indexingTime);
    QString getTitleId(int pageID);
    QString getBookName(int bookID);

public slots:
    void startIndexing();
    void startSearching();
    void showStatistic();
    void indexDocs(IndexWriter* writer);
    Document* FileDocument(const QString &id, const QString &ayaText);
    QString cleanString(QString str);
    void resultsCount();
    void displayResults(/*result &pResult*/);
    void setPageCount(int current, int count);
    void buttonStat(int currentPage, int pageCount);
    QStringList makeVLabels(int start, int end);
    void openDB();
    void setResultParPage(int count){m_resultParPage = count;}
    QString buildFilePath(QString bkid);

protected:
    QSqlDatabase m_bookDB;
    QSqlQuery *m_bookQuery;
    QString m_quranDBPath;
    QString m_titleName;
    QString m_bookTableName;
    QString m_bookPath;
    QString m_bookName;
    QString m_searchQuery;
    QString m_highLightRE;
    QStandardItemModel *m_resultModel;
    QList<QString> m_colors;
    result m_resultStruct;
    int m_resultCount;
    int m_resultParPage;
    bool m_dbIsOpen;
    bool m_haveMainTable;
    Ui::MainWindow *ui;

private slots:
    void on_pushButton_2_clicked();
    void resultLinkClicked(const QUrl &url);
    void on_pushButton_clicked();
    void on_lineQuery_returnPressed();
    void on_pushGoLast_clicked();
    void on_pushGoFirst_clicked();
    void on_pushGoPrev_clicked();
    void on_pushGoNext_clicked();
};

#endif // MAINWINDOW_H
