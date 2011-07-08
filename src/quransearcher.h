#ifndef QURANSEARCHER_H
#define QURANSEARCHER_H

#include "cl_common.h"
#include <qthread.h>
#include <qsqldatabase.h>

class IndexInfo;
class BooksDB;
class QuranResult;
class ArabicAnalyzer;

class QuranSearcher : public QThread
{
    Q_OBJECT

    enum Action {
        SEARCH,
        FETECH
    };

public:
    QuranSearcher(QObject *parent = 0);
    ~QuranSearcher();
    void setSearchText(QString text);
    void setBooksDb(BooksDB *db);
    void setIndexInfo(IndexInfo *index);
    void setResultsPeerPage(int count);

    int currentPage() { return m_currentPage; }
    int pagesCount() { return m_pageCount; }
    int searchTime() { return m_timeSearch; }
    int resultCount() { return m_resultCount; }
    int resultsPeerPage() { return m_resultParPage; }

    void run();
    void search();
    void fetech();

public slots:
//    void stopFeteching();
    void nextPage();
    void prevPage();
    void firstPage();
    void lastPage();
    void fetechResults(int page);

signals:
    void startSearching();
    void doneSearching();
    void startFeteching();
    void doneFeteching();
    void gotResult(QuranResult *result);
    void gotException(QString what, int id);

protected:
    Action m_action;
    QString m_queryText;
    IndexInfo *m_indexInfo;
    BooksDB *m_booksDb;
    IndexSearcher *m_searcher;
    ArabicAnalyzer *m_analyzer;
    QueryParser *m_queryPareser;
    Hits* m_hits;
    Query* m_query;
    int m_resultCount;
    int m_currentPage;
    int m_pageCount;
    int m_timeSearch;
    int m_resultParPage;
    bool m_init;
};

#endif // QURANSEARCHER_H
