#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QFileDialog>
#include <QSettings>
#include <QSpinBox>
#include <QStandardItemModel>
#include "common.h"
#include "arabicanalyzer.h"
#include "indexingdialg.h"
#include "indexthread.h"

namespace Ui {
    class MainWindow;
}

class Results;

class MainWindow : public QMainWindow
{
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
    QString cleanString(QString str);
    void resultsCount();
    void displayResults(/*result &pResult*/);
    void setPageCount(int current, int count);
    void buttonStat(int currentPage, int pageCount);
    bool openDB();
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
    Results *m_results;
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

class Results
{
public:
    Results(){};
    int idAt(int index){ return FIELD_TO_INT("id", (&m_hits->doc(index))); }
    int bookIdAt(int index){ return FIELD_TO_INT("bookid", (&m_hits->doc(index))); }
    float_t scoreAt(int index) { return m_hits->score(index); }

    int pageCount() { return m_pageCount; }
    int currentPage() { return m_page; }
    void setPageCount(int pageCount) { m_pageCount = pageCount; }
    void setCurrentPage(int page) { m_page = page; }
    void setHits(Hits *hit) { m_hits = hit; }
    int resultsCount() { return m_hits->length(); }
private:
    Hits* m_hits;
    int m_page;
    int m_pageCount;
};

#endif // MAINWINDOW_H
