#include "quransearcher.h"
#include "indexinfo.h"
#include "booksdb.h"
#include "common.h"
#include "arabicanalyzer.h"
#include "quranresult.h"
#include <QTime>
#include <qvariant.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlerror.h>

QuranSearcher::QuranSearcher(QObject *parent) : QThread(parent)
{
    m_action = SEARCH;
    m_resultParPage = 10;
    m_init = false;
    m_searcher = 0;
    m_analyzer = 0;
    m_queryPareser = 0;
}

QuranSearcher::~QuranSearcher()
{
    if(m_searcher)
        delete m_searcher;
    if(m_analyzer)
        delete m_analyzer;
    if(m_queryPareser)
        delete m_queryPareser;
}

void QuranSearcher::run()
{
    try {
        if(m_action == SEARCH) {
            search();
            if(m_resultCount > 0)
                fetech();
        } else if(m_action == FETECH) {
            fetech();
        }

    } catch(CLuceneError &e) {
        qCritical("Error when searching: %s\ncode: %d", e.what(), e.number());
        emit gotException(e.what(), e.number());
    }
    catch(std::exception &e){
        emit gotException(e.what(), 0);
    }
    catch(...) {
        qCritical("Unknow error when searching at \"%s\".", qPrintable(m_indexInfo->path()));
        emit gotException("UNKNOW", -1);
    }
}

void QuranSearcher::setSearchText(QString text)
{
    m_queryText = text;
    m_action = SEARCH;
}

void QuranSearcher::setBooksDb(BooksDB *db)
{
    m_booksDb = db;
}

void QuranSearcher::setIndexInfo(IndexInfo *index)
{
    m_indexInfo = index;
}

void QuranSearcher::search()
{
    emit startSearching();

    if(!m_init) {
        m_searcher = new IndexSearcher(qPrintable(m_indexInfo->indexPath()));
        m_analyzer = new ArabicAnalyzer();
        m_queryPareser = new QueryParser(_T("text"), m_analyzer);

        m_init = true;
    }

    m_query = m_queryPareser->parse(QStringToTChar(m_queryText));
    qDebug() << "Search for:" << TCharToQString(m_query->toString(_T("text")));

    QTime time;
    time.start();
    m_hits = m_searcher->search(m_query);
    m_timeSearch = time.elapsed();

    m_resultCount = m_hits->length();
    m_pageCount = ceil((m_resultCount/(double)m_resultParPage));
    m_currentPage = 0;

    emit doneSearching();
}

void QuranSearcher::fetech()
{
    emit startFeteching();

    ArabicAnalyzer hl_analyzer;
    QueryScorer scorer(m_query->rewrite(IndexReader::open(qPrintable(m_indexInfo->indexPath()))));
    SimpleCssFormatter hl_formatter;
    int maxNumFragmentsRequired = 30;
    const TCHAR* fragmentSeparator = _T("...");

    int start = m_currentPage * m_resultParPage;
    int maxResult  =  (m_resultCount >= start+m_resultParPage)
                      ? (start+m_resultParPage) : m_resultCount;

    {
        QSqlDatabase bookDB = QSqlDatabase::addDatabase("QODBC", "quran__");

        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(m_indexInfo->shamelaSpecialDbPath());
        bookDB.setDatabaseName(mdbpath);

        if (!bookDB.open()) {
            qDebug("emit gotResult(result);");
            DB_OPEN_ERROR(m_indexInfo->shamelaSpecialDbPath());
            return;
        }

        QSqlQuery bookQuery(bookDB);

        bool whiteBG = false;
        for(int i=start; i < maxResult;i++){

            Document &doc = m_hits->doc(i);
            int entryID = _wtoi(doc.get(_T("id")));
            int soraNumber = _wtoi(doc.get(_T("sora")));
            int score = (int) (m_hits->score(i) * 100.0);
            /*
            qDebug() << TCharToQString(doc.get(_T("text")));
            int bookID = _wtoi(doc.get(_T("bookid")));
            qDebug("Result at %d == %d", i, entryID);
            */

            if(!bookQuery.exec(QString("SELECT nass, Page, sora, aya FROM Qr WHERE Id = %1")
                               .arg(entryID)))
                SQL_ERROR(bookQuery.lastError().text());

            if(bookQuery.first()) {
                QuranResult *result = new QuranResult();
                QString pageText(bookQuery.value(0).toString());

                result->setId(i);
                result->setPageId(entryID);
                result->setPage(bookQuery.value(1).toInt());
                result->setAya(bookQuery.value(3).toInt());
                result->setSoraName(m_booksDb->getSoraName(soraNumber));
                result->setSoraNumber(soraNumber);
                result->setText(pageText);
                result->setScore(score);
                result->setBgColor((whiteBG = !whiteBG) ? "whiteBG" : "grayBG");

                /* Highlight the text */
                Highlighter highlighter(&hl_formatter, &scorer);
                SimpleFragmenter frag(20);
                highlighter.setTextFragmenter(&frag);

                const TCHAR* text = QStringToTChar(pageText);
                StringReader reader(text);
                TokenStream* tokenStream = hl_analyzer.tokenStream(_T("text"), &reader);

                TCHAR* hi_result = highlighter.getBestFragments(
                            tokenStream,
                            text,
                            maxNumFragmentsRequired,
                            fragmentSeparator);

                result->setSnippet(TCharToQString(hi_result));

                _CLDELETE_CARRAY(hi_result);
                _CLDELETE(tokenStream);
                delete [] text;
                /**/
                emit gotResult(result);

            } else {
                qDebug("No result");
            }
        }
    }

    QSqlDatabase::removeDatabase("quran__");

    emit doneFeteching();
}


void QuranSearcher::nextPage()
{
    if(!isRunning()) {
        if(m_currentPage+1 < m_pageCount) {
            m_currentPage++;
            m_action = FETECH;

            start();
        }
    }
}

void QuranSearcher::prevPage()
{
    if(!isRunning()) {
        if(m_currentPage-1 >= 0) {
            m_currentPage--;
            m_action = FETECH;

            start();
        }
    }
}

void QuranSearcher::firstPage()
{
    if(!isRunning()) {
        m_currentPage=0;
        m_action = FETECH;

        start();
    }
}

void QuranSearcher::lastPage()
{
    if(!isRunning()) {
        m_currentPage = m_pageCount-1;
        m_action = FETECH;

        start();
    }
}

void QuranSearcher::fetechResults(int page)
{
    if(!isRunning()) {
        if(page < m_pageCount) {
            m_currentPage = page;
            m_action = FETECH;

            start();
        }
    }
}

void QuranSearcher::setResultsPeerPage(int count)
{
    m_resultParPage = count;
}

