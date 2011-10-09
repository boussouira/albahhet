#include "shamelasearcher.h"
#include "arabicanalyzer.h"
#include "common.h"
#include "cl_common.h"
#include "booksdb.h"
#include "bookinfo.h"
#include "indexinfo.h"
#include "shamelaresult.h"
#include <cmath>
#include <exception>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qdebug.h>
#include <qvariant.h>

ShamelaSearcher::ShamelaSearcher(QObject *parent) : QThread(parent)
{
    m_hits = 0;
    m_query = 0;
    m_orignalQuery = 0;
    m_searcher = 0;
    m_currentPage = 0;
    m_pageCount = 0;
    m_action = SEARCH;
    m_defautOpIsAnd = false;
    m_stopFeteching = false;
    m_resultParPage = 10;
    m_timeSearch = 0;

    m_sort = new Sort();
}

ShamelaSearcher::~ShamelaSearcher()
{
    clear();
}

void ShamelaSearcher::run()
{
    try {
        if(m_action == SEARCH){
            search();

            if(m_hits->length() > 0)
                fetech();

        } else if (m_action == FETECH) {
            fetech();
        }
    } catch(CLuceneError &e) {
        qCritical("Search error: %s", e.what());
        emit gotException(e.what(), e.number());
    }
    catch(std::exception &e){
        emit gotException(e.what(), 0);
    }
    catch(...) {
        qCritical("Unknow error when searching at \"%s\".", qPrintable(m_indexInfo->indexPath()));
        emit gotException("UNKNOW", -1);
    }

}

void ShamelaSearcher::search()
{
    emit startSearching();

    qDeleteAll(m_resultsHash);
    m_resultsHash.clear();

    m_searcher = new IndexSearcher(qPrintable(m_indexInfo->indexPath()));

    m_query = m_searcher->rewrite(m_orignalQuery);
//    qDebug() << "Search [Orig]:" << TCharToQString(m_orignalQuery->toString(PAGE_TEXT_FIELD));
//    qDebug() << "Search for:" << TCharToQString(m_query->toString(PAGE_TEXT_FIELD));

    QTime time;
    time.start();

    SortField *sort1[] = {SortField::FIELD_SCORE(), NULL};
    SortField *sort2[] = {new SortField(BOOK_ID_FIELD), SortField::FIELD_SCORE(), NULL};
    SortField *sort3[] = {new SortField(BOOK_ID_FIELD), new SortField(PAGE_ID_FIELD), NULL};
    SortField *sort4[] = {new SortField(AUTHOR_DEATH_FIELD), SortField::FIELD_SCORE(), NULL};
    SortField *sort5[] = {new SortField(AUTHOR_DEATH_FIELD), new SortField(BOOK_ID_FIELD), new SortField(PAGE_ID_FIELD), NULL};

    QList<SortField**> sortFields;
    sortFields << sort1;
    sortFields << sort2;
    sortFields << sort3;
    sortFields << sort4;
    sortFields << sort5;

    m_sort->setSort(sortFields.at(m_sortNum));

    m_hits = m_searcher->search(m_query, m_sort);

    m_timeSearch = time.elapsed();

    m_pageCount = ceil((resultsCount()/(double)m_resultParPage));
    m_currentPage = 0;

//    foreach(SortField** f, m_sortFields) {
//        delete[] f;
//    }

    emit doneSearching();
}

void ShamelaSearcher::fetech()
{
    emit startFeteching();

    ArabicAnalyzer hl_analyzer;
    QueryScorer scorer(m_query);
    SimpleCssFormatter hl_formatter;
    int maxNumFragmentsRequired = 30;
    const TCHAR* fragmentSeparator = _T("...");

    int start = m_currentPage * m_resultParPage;
    int maxResult  =  (resultsCount() >= start+m_resultParPage)
                      ? (start+m_resultParPage) : resultsCount();

    bool whiteBG = false;
    for(int i=start; i < maxResult;i++){

        ShamelaResult *savedResult = m_resultsHash.value(i, 0);
        if(savedResult) {
            emit gotResult(savedResult);
            continue;
        }

        Document &doc = m_hits->doc(i);
        int entryID = _wtoi(doc.get(PAGE_ID_FIELD));
        int bookID = _wtoi(doc.get(BOOK_ID_FIELD));
        int score = (int) (m_hits->score(i) * 100.0);

        BookInfo *bookInfo = m_booksDb->getBookInfo(bookID);

        if(!bookInfo) {
            qCritical("ShamelaSearcher::fetech: No book with id %d where found", bookID);
            continue;
        }

        QString connName = (bookInfo->archive()) ? QString("bid_%1").arg(bookInfo->archive()) :
                           QString("bid_%1_%2").arg(bookInfo->archive()).arg(bookID);

        {
            QSqlDatabase bookDB;
            if(bookInfo->archive() && QSqlDatabase::contains(connName)) {
                bookDB = QSqlDatabase::database(connName);
            } else {
                bookDB = QSqlDatabase::addDatabase("QODBC", connName);
                QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                  .arg(bookInfo->path());
                bookDB.setDatabaseName(mdbpath);
            }

            if (!bookDB.open()) {
                DB_OPEN_ERROR(bookInfo->path());
                continue;
            }

            QSqlQuery bookQuery(bookDB);

            bookQuery.exec(QString("SELECT nass, page, part FROM %1 WHERE id = %2")
                             .arg(bookInfo->mainTable())
                             .arg(entryID));
            if(bookQuery.first()){
                ShamelaResult *result = new ShamelaResult;
                QString pageText(bookQuery.value(0).toString());

                QList<QPair<QString, QString> > shoorts;
                shoorts = m_booksDb->getBookShoorts(bookInfo->id());

                for(int j=0; j < shoorts.count(); j++)
                    pageText.replace(shoorts.at(j).first, j ? ' ' + shoorts.at(j).second : shoorts.at(j).second);

                result->setId(i);
                result->setBookId(bookID);
                result->setArchive(bookInfo->archive());
                result->setPageId(entryID);
                result->setBookName(bookInfo->name());
                result->setPage(bookQuery.value(1).toInt());
                result->setPart(bookQuery.value(2).toInt());
                result->setScore(score);
                result->setTitle(getTitleId(bookDB, result));
                result->setBgColor((whiteBG = !whiteBG) ? "whiteBG" : "grayBG");

                /* Highlight the text */
                Highlighter highlighter(&hl_formatter, &scorer);
                SimpleFragmenter frag(20);
                highlighter.setTextFragmenter(&frag);

                clearShorts(pageText);

                const TCHAR* text = QStringToTChar(pageText);
                StringReader reader(text);
                TokenStream* tokenStream = hl_analyzer.tokenStream(PAGE_TEXT_FIELD, &reader);

                TCHAR* hi_result = highlighter.getBestFragments(
                        tokenStream,
                        text,
                        maxNumFragmentsRequired,
                        fragmentSeparator);

                result->setSnippet(TCharToQString(hi_result));

                _CLDELETE_CARRAY(hi_result)
                _CLDELETE(tokenStream)
                delete [] text;
                /**/

                        emit gotResult(result);
                m_resultsHash.insert(i, result);
            } else {
                qWarning("No result found for id %d book %d", entryID, bookID);
            }
        }

        QSqlDatabase::removeDatabase(connName);

        if(m_stopFeteching) {
            m_stopFeteching = false;
            break;
        }
    }

    emit doneFeteching();
}

void ShamelaSearcher::clear()
{
    if(m_hits)
        _CLDELETE(m_hits);

    if(m_query)
        _CLDELETE(m_query);

//    if(m_orignalQuery)
//        _CLDELETE(m_orignalQuery);

    if(m_searcher) {
        m_searcher->close();
        _CLDELETE(m_searcher);
    }

    _CLDELETE(m_sort);

    qDeleteAll(m_resultsHash);
    m_resultsHash.clear();

    m_currentPage = 0;
    m_pageCount = 0;
    m_timeSearch = 0;
}

int ShamelaSearcher::pageCount()
{
    return m_pageCount;
}

int ShamelaSearcher::currentPage()
{
    return m_currentPage;
}

void ShamelaSearcher::setIndexInfo(IndexInfo *index)
{
    m_indexInfo = index;
}

void ShamelaSearcher::setBooksDb(BooksDB *db)
{
    m_booksDb = db;
}

void ShamelaSearcher::setsetDefaultOperator(bool defautIsAnd)
{
    m_defautOpIsAnd = defautIsAnd;
}

void ShamelaSearcher::setPageCount(int pageCount)
{
    m_pageCount = pageCount;
}

void ShamelaSearcher::setCurrentPage(int page)
{
    m_currentPage = page;
}

void ShamelaSearcher::setHits(Hits *hit)
{
    m_hits = hit;
}

void ShamelaSearcher::setQuery(Query* q)
{
    m_orignalQuery = q;
}

void ShamelaSearcher::setQueryString(QString q)
{
    m_queryStr = q;
}

void ShamelaSearcher::setSearcher(IndexSearcher *searcher)
{
    m_searcher = searcher;
}

int ShamelaSearcher::resultsCount()
{
    return m_hits->length();
}

QString ShamelaSearcher::getTitleId(const QSqlDatabase &db, ShamelaResult *result)
{
    QSqlQuery m_bookQuery(db);
    bool exec;

    exec = m_bookQuery.exec(QString("SELECT TOP 1 tit FROM %1 WHERE id <= %2 ORDER BY id DESC")
                            .arg((!result->archive()) ? "title" : QString("t%1").arg(result->bookId()))
                            .arg(result->pageID()));

    if(!exec)
        SQL_ERROR(m_bookQuery.lastError().text());

    return m_bookQuery.first() ? m_bookQuery.value(0).toString() : QString();
}

void ShamelaSearcher::nextPage()
{
    if(!isRunning()) {
        if(m_currentPage+1 < pageCount()) {
            m_currentPage++;
            m_action = FETECH;

            start();
        }
    }
}

void ShamelaSearcher::prevPage()
{
    if(!isRunning()) {
        if(m_currentPage-1 >= 0) {
            m_currentPage--;
            m_action = FETECH;

            start();
        }
    }
}

void ShamelaSearcher::firstPage()
{
    if(!isRunning()) {
        m_currentPage=0;
        m_action = FETECH;

        start();
    }
}

void ShamelaSearcher::lastPage()
{
    if(!isRunning()) {
        m_currentPage = pageCount()-1;
        m_action = FETECH;

        start();
    }
}

void ShamelaSearcher::fetechResults(int page)
{
    if(!isRunning()) {
        if(page < pageCount()) {
            m_currentPage = page;
            m_action = FETECH;

            start();
        }
    }
}

ShamelaResult * ShamelaSearcher::getSavedResult(int resultID)
{
    return m_resultsHash.value(resultID, 0);
}

void ShamelaSearcher::stopFeteching()
{
    m_stopFeteching = true;
}
