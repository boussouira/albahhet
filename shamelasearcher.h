#ifndef SHAMELASEARCHER_H
#define SHAMELASEARCHER_H

#include "common.h"
#include "indexinfo.h"
#include "arabicanalyzer.h"
#include "shamelaresult.h"
#include <qthread.h>
#include <qdatetime.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qdebug.h>
#include <qvariant.h>

class ShamelaSearcher : public QThread
{
    Q_OBJECT

public:
    ShamelaSearcher(QObject *parent = 0);
    ~ShamelaSearcher();
    enum Action {
        SEARCH,
        FETECH
    };

    void run();
    void clear();

    int idAt(int index);
    int bookIdAt(int index);
    int ArchiveAt(int index);
    float_t scoreAt(int index);

    int pageCount();
    int currentPage();
    int resultsCount();
    int searchTime() { return m_timeSearch;}
    int resultsPeerPage() { return m_resultParPage;}
    QString queryString() { return m_queryStr; }

    void setIndexInfo(IndexInfo *index);
    void setPageCount(int pageCount);
    void setCurrentPage(int page);
    void setHits(Hits *hit);
    void setQuery(Query* q);
    void setQueryString(QString q);
    void setSearcher(IndexSearcher *searcher);
    void setsetDefaultOperator(bool DefautIsAnd);
    void setResultsPeerPage(int count) { m_resultParPage = count; }

    QString buildFilePath(QString bkid, int archive);
    QStringList buildRegExp(const QString &str);
    QString cleanString(QString str);
    QString abbreviate(QString str, int size);
    QString hiText(const QString &text, const QString &strToHi);
    QString getTitleId(const QSqlDatabase &db, ShamelaResult *result);

protected:
    void search();
    void fetech();

public slots:
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
    IndexInfo *m_indexInfo;
    Hits* m_hits;
    Query* m_query;
    QString m_queryStr;
    IndexSearcher *m_searcher;
    int m_currentPage;
    int m_pageCount;
    bool m_defautOpIsAnd;
    int m_timeSearch;
    int m_resultParPage;
    QList<QString> m_colors;
};

#endif // SHAMELASEARCHER_H
