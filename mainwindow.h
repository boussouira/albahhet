#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>

#include <CLucene/StdHeader.h>
#include <CLucene.h>
#include <CLucene/util/CLStreams.h>
#include <CLucene/util/Misc.h>
#include <CLucene/config/repl_tchar.h>
#include <CLucene/util/StringBuffer.h>
#include <CLucene/util/dirent.h>
#include <CLucene/search/IndexSearcher.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cctype>
#include <string.h>
#include <algorithm>

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::queryParser;
using namespace lucene::document;
using namespace lucene::search;

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

public slots:
    void startIndexing();
    void startSearching();
    void showStatistic();
    void indexDocs(IndexWriter* writer);
    Document* FileDocument(QString id, QString ayaText);
    QString cleanString(QString str);
    void resultsCount();

protected:
    QSqlDatabase m_quranDB;
    QSqlQuery *m_quranQuery;
    QString m_quranDBPath;
    QSqlQueryModel *m_resultModel;
    Ui::MainWindow *ui;
    QMap<QString, QChar> letterMap;
};

#endif // MAINWINDOW_H
