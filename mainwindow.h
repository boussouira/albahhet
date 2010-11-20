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
    QString getBooksSize();
    qint64 getDirSize(const QString &path);
    void doneIndexing(int indexingTime);
    QString getTitleId(int pageID, int archive, int bookID);
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
    QString buildFilePath(QString bkid, int archive);

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
    Results()
    {
        m_hits = NULL;
        m_query = NULL;
        m_searcher = NULL;
        m_page = 0;
        m_pageCount = 0;
    }
    void clear()
    {
        if(m_hits != NULL)
            _CLDELETE(m_hits)

        if(m_query != NULL)
            _CLDELETE(m_query)

        if(m_searcher != NULL) {
            m_searcher->close();
            _CLDELETE(m_searcher)
        }

        m_page = 0;
        m_pageCount = 0;
    }

    int idAt(int index){ return FIELD_TO_INT("id", (&m_hits->doc(index))); }
    int bookIdAt(int index){ return FIELD_TO_INT("bookid", (&m_hits->doc(index))); }
    int ArchiveAt(int index){ return FIELD_TO_INT("archive", (&m_hits->doc(index))); }
    float_t scoreAt(int index) { return m_hits->score(index); }

    int pageCount() { return m_pageCount; }
    int currentPage() { return m_page; }
    void setPageCount(int pageCount) { m_pageCount = pageCount; }
    void setCurrentPage(int page) { m_page = page; }
    void setHits(Hits *hit) { m_hits = hit; }
    void setQuery(Query* q) { m_query = q; }
    void setSearcher(IndexSearcher *searcher) { m_searcher = searcher; }
    int resultsCount() { return m_hits->length(); }

private:
    Hits* m_hits;
    Query* m_query;
    IndexSearcher *m_searcher;
    int m_page;
    int m_pageCount;
};

#endif // MAINWINDOW_H
