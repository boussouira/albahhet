#include "shamelasearcher.h"

ShamelaSearcher::ShamelaSearcher(QObject *parent) : QThread(parent)
{
    m_hits = NULL;
    m_query = NULL;
    m_searcher = NULL;
    m_currentPage = 0;
    m_pageCount = 0;
    m_action = SEARCH;
    m_defautOpIsAnd = false;
    m_resultParPage = 10;
    m_timeSearch = 0;

    m_colors.append("#FFFF63");
    m_colors.append("#A5FFFF");
    m_colors.append("#FF9A9C");
    m_colors.append("#9CFF9C");
    m_colors.append("#EF86FB");
}

void ShamelaSearcher::run()
{
    if(m_action == SEARCH){
        qDebug("Start Searching...");
        search();
        fetech();
    } else if (m_action == FETECH) {
        qDebug("Start Feteching...");
        fetech();
    }
}

void ShamelaSearcher::search()
{
    emit startSearching();

    try {
        ArabicAnalyzer analyzer;
        IndexSearcher *searcher = new IndexSearcher(qPrintable(m_indexInfo->path()));

        // Start building the query
        QueryParser *queryPareser = new QueryParser(_T("text"),&analyzer);
        queryPareser->setAllowLeadingWildcard(true);

        if(m_defautOpIsAnd)
            queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);

        Query* q = queryPareser->parse(QSTRING_TO_TCHAR(m_queryStr));
//        qDebug() << "Search: " << TCHAR_TO_QSTRING(q->toString(_T("text")));
//        qDebug() << "Query : " << m_queryStr;

        QTime time;

        time.start();
        setHits(searcher->search(q));
        m_timeSearch = time.elapsed();

        m_pageCount = _ceil((resultsCount()/(double)m_resultParPage));
        m_currentPage = 0;

        setQuery(q);
        setSearcher(searcher);

    } catch(CLuceneError &tmp) {
        qDebug() << "Error when searching" << tmp.what();
    } catch(...) {
        qDebug() << "Error when searching at : " << m_indexInfo->path();
    }

    emit doneSearching();
}

void ShamelaSearcher::fetech()
{
    emit startFeteching();

    int start = m_currentPage * m_resultParPage;
    int maxResult  =  (resultsCount() >= start+m_resultParPage)
                      ? (start+m_resultParPage) : resultsCount();
    int entryID;
    bool whiteBG = false;
    for(int i=start; i < maxResult;i++){
        //Document doc = m_hits->doc(i);
        int bookID = this->bookIdAt(i);
        int archive = this->ArchiveAt(i);
        int score = (int) (this->scoreAt(i) * 100.0);

        QString connName = (archive) ? QString("bid_%1").arg(archive) : QString("bid_%1_%2").arg(archive).arg(bookID);

        {
            QSqlDatabase bookDB;
            if(archive && QSqlDatabase::contains(connName)) {
                bookDB = QSqlDatabase::database(connName);
            } else {
                bookDB = QSqlDatabase::addDatabase("QODBC", connName);
                QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                  .arg(buildFilePath(QString::number(bookID), archive));
                bookDB.setDatabaseName(mdbpath);
            }

            if (!bookDB.open())
                qDebug() << "Cannot open" << buildFilePath(QString::number(bookID), archive) << "database.";

            QSqlQuery bookQuery(bookDB);

            entryID = this->idAt(i);
            bookQuery.exec(QString("SELECT nass, page, part FROM %1 WHERE id = %2")
                             .arg((!archive) ? "book" : QString("b%1").arg(bookID))
                             .arg(entryID));
            if(bookQuery.first()){
                ShamelaResult *result = new ShamelaResult;
                result->setBookId(bookID);
                result->setArchive(archive);
                result->setId(entryID);
                result->setPage(bookQuery.value(1).toInt());
                result->setPart(bookQuery.value(2).toInt());
                result->setScore(score);
                result->setText(bookQuery.value(0).toString());
                result->setTitle(getTitleId(bookDB, result));
                result->setSnippet(hiText(abbreviate(result->text(), 320), m_queryStr));
                result->setBgColor((whiteBG = !whiteBG) ? "whiteBG" : "grayBG");

                emit gotResult(result);
            }
        }
        if(!archive)
            QSqlDatabase::removeDatabase(connName);
    }

    emit doneFeteching();
}

void ShamelaSearcher::clear()
{
    if(m_hits != NULL)
        _CLDELETE(m_hits)

    if(m_query != NULL)
        _CLDELETE(m_query)

    if(m_searcher != NULL) {
        m_searcher->close();
        _CLDELETE(m_searcher)
    }

    m_currentPage = 0;
    m_pageCount = 0;
    m_timeSearch = 0;
}

int ShamelaSearcher::idAt(int index)
{
    return FIELD_TO_INT("id", (&m_hits->doc(index)));
}

int ShamelaSearcher::bookIdAt(int index)
{
    return FIELD_TO_INT("bookid", (&m_hits->doc(index)));
}

int ShamelaSearcher::ArchiveAt(int index)
{
    return FIELD_TO_INT("archive", (&m_hits->doc(index)));
}

float_t ShamelaSearcher::scoreAt(int index)
{
    return m_hits->score(index);
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
    m_query = q;
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

QString ShamelaSearcher::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(m_indexInfo->shamelaPath()).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(m_indexInfo->shamelaPath()).arg(archive);
}

QStringList ShamelaSearcher::buildRegExp(const QString &str)
{
QStringList strWords = str.split(QRegExp(trUtf8("[\\s;,.()\"'{}\\[\\]]")), QString::SkipEmptyParts);
QStringList regExpList;
    QChar opPar('(');
    QChar clPar(')');
    foreach(QString word, strWords)
    {
        QString regExpStr;
        regExpStr.append("\\b");
        regExpStr.append(opPar);

        for (int i=0; i< word.size();i++) {
            if(word.at(i) == QChar('~'))
                regExpStr.append("[\\S]*");
            else if(word.at(i) == QChar('*'))
                regExpStr.append("[\\S]*");
            else if(word.at(i) == QChar('?'))
                regExpStr.append("\\S");
            else if( word.at(i) == QChar('"') || word.at(i) == opPar || word.at(i) == opPar )
                continue;
            else {
                regExpStr.append(word.at(i));
                regExpStr.append(trUtf8("[ًٌٍَُِّْ]*"));
            }
        }

        regExpStr.append(clPar);
        regExpStr.append("\\b");
        regExpList.append(regExpStr);
    }

    return regExpList;
}

QString ShamelaSearcher::abbreviate(QString str, int size) {
        if (str.length() <= size-3)
                return str;
        str.simplified();
        int index = str.lastIndexOf(' ', size-3);
        if (index <= -1)
                return "";
        return str.left(index).append("...");
}

QString ShamelaSearcher::cleanString(QString str)
{
    str.replace(QRegExp("[\\x0627\\x0622\\x0623\\x0625]"), "[\\x0627\\x0622\\x0623\\x0625]");//ALEFs
    str.replace(QRegExp("[\\x0647\\x0629]"), "[\\x0647\\x0629]"); //TAH_MARBUTA -> HEH

    return str;
}

QString ShamelaSearcher::hiText(const QString &text, const QString &strToHi)
{
    QStringList regExpStr = buildRegExp(strToHi);
    QString finlStr  = text;
    int color = 0;
    bool useColors = (regExpStr.size() <= m_colors.size());

    foreach(QString regExp, regExpStr)
        finlStr.replace(QRegExp(cleanString(regExp)),
                        QString("<b style=\"background-color:%1\">\\1</b>")
                        .arg(m_colors.at(useColors ? color++ : color)));

//    if(!useColors)
//        finlStr.replace(QRegExp("<\\/b>([\\s])<b style=\"background-color:[^\"]+\">"), "\\1");

    return finlStr;
}

QString ShamelaSearcher::getTitleId(const QSqlDatabase &db, ShamelaResult *result)
{
    QSqlQuery m_bookQuery(db);
    m_bookQuery.exec(QString("SELECT TOP 1 tit FROM %1 WHERE id <= %2 ORDER BY id DESC")
                     .arg((!result->archive()) ? "title" : QString("t%1").arg(result->bookId()))
                     .arg(result->id()));

    if(m_bookQuery.first())
        return m_bookQuery.value(0).toString();
    else {
        qDebug() << m_bookQuery.lastError().text();
        return QString();
    }

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
