#ifndef SHAMELASEARCHER_H
#define SHAMELASEARCHER_H

#include "cl_common.h"
#include <qthread.h>
#include <qsqldatabase.h>
#include <qhash.h>

class ShamelaIndexInfo;
class BooksDB;
class ShamelaResult;

class ShamelaSearcher : public QThread
{
    Q_OBJECT

public:
    enum Action {
        SEARCH,
        FETECH
    };

    ShamelaSearcher(QObject *parent = 0);
    ~ShamelaSearcher();

    void run();
    void clear();

    int pageCount();
    int currentPage();
    int resultsCount();
    int searchTime() { return m_timeSearch;}
    int resultsPeerPage() { return m_resultParPage;}
    QString queryString() { return m_queryStr; }

    void setBooksDb(BooksDB *db);
    void setIndexInfo(ShamelaIndexInfo *index);
    void setPageCount(int pageCount);
    void setCurrentPage(int page);
    void setHits(Hits *hit);
    void setQuery(Query* q);
    void setQueryString(QString q);
    void setSearcher(IndexSearcher *searcher);
    void setsetDefaultOperator(bool DefautIsAnd);
    void setResultsPeerPage(int count) { m_resultParPage = count; }
    void setSortNum(SearchSort sortNum) { m_sortNum = sortNum; }

    QString getTitleId(const QSqlDatabase &db, ShamelaResult *result);
    ShamelaResult *getSavedResult(int resultID);
protected:
    void search();
    void fetech();

public slots:
    void stopFeteching();
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
    void gotResult(ShamelaResult *result);
    void gotException(QString what, int id);


private:
    Action m_action;
    ShamelaIndexInfo *m_indexInfo;
    BooksDB *m_booksDb;
    Hits* m_hits;
    Query* m_query;
    Query* m_orignalQuery;
    QString m_queryStr;
    IndexSearcher *m_searcher;
    Sort *m_sort;
    int m_currentPage;
    int m_pageCount;
    int m_timeSearch;
    int m_resultParPage;
    bool m_defautOpIsAnd;
    bool m_stopFeteching;
    bool m_highlightAllPage;
    QHash<int, ShamelaResult*> m_resultsHash;
    SearchSort m_sortNum;
};

#endif // SHAMELASEARCHER_H
