#include "shamelaindexer.h"
#include "common.h"
#include "booksdb.h"
#include "indexinfo.h"
#include "bookinfo.h"
#include <qvariant.h>
#include <qsqlquery.h>
#include <qmessagebox.h>

ShamelaIndexer::ShamelaIndexer()
{
    m_skipCurrent = false;
}

void ShamelaIndexer::run()
{
    startIndexing();
}

void ShamelaIndexer::startIndexing()
{
    try {
        BookInfo *book = m_bookDB->next();
        while(book != NULL) {
            emit currentBookName(book->name());
            indexBook(book);
            _CLDELETE(book);

            book = m_bookDB->next();
        }
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "CLucene Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
        emit indexingError();
        terminate();
    }
    catch(std::exception &err){
        QMessageBox::warning(0, "Error when Indexing",
                             tr("exception: %1").arg(err.what()));
        emit indexingError();
        terminate();
    }
    catch(...){
        QMessageBox::warning(0, "Unkonw error when Indexing",
                             tr("Unknow error"));
        emit indexingError();
        terminate();
    }
}

void ShamelaIndexer::indexBook(BookInfo *book)
{
    {
        QSqlDatabase mdbDB = QSqlDatabase::addDatabase("QODBC", QString("INDEX_%1").arg(book->id()));
        mdbDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                 .arg(book->path()));

        if (!mdbDB.open()) {
            DB_OPEN_ERROR(book->path());
            return;
        }

        book->genInfo();

        QSqlQuery shaQuery(mdbDB);
        shaQuery.exec(QString("SELECT MAX(id) FROM %1").arg(book->mainTable()));

        if(shaQuery.next())
            emit currentBookMax(shaQuery.value(0).toInt());

        shaQuery.exec(QString("SELECT id, nass FROM %1 ORDER BY id ").arg(book->mainTable()));

        Document doc;
        int indexedPages = 0;
        int tokenAndNoStore = Field::STORE_NO | Field::INDEX_TOKENIZED;
        int storeAndNoToken = Field::STORE_YES | Field::INDEX_UNTOKENIZED;

        while(shaQuery.next())
        {
           doc.add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(shaQuery.value(0).toString()),
                                   storeAndNoToken));
            doc.add( *_CLNEW Field(_T("bookid"), book->idT(), storeAndNoToken));
            doc.add( *_CLNEW Field(_T("cat"), book->catT(), tokenAndNoStore));
            doc.add( *_CLNEW Field(_T("author"), book->authorIDT(), tokenAndNoStore));
            doc.add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(shaQuery.value(1).toString()),
                                   tokenAndNoStore));

            m_writer->addDocument(&doc);

            doc.clear();

            if(indexedPages > 500) {
                emit currentBookProgress(shaQuery.value(0).toInt());
                indexedPages = 0;
            } else {
                indexedPages++;
            }

            if(m_skipCurrent) {
                m_skipCurrent = false;
                break;
            }
        }

        mdbDB.close();
    }

    m_bookDB->setBookIndexed(book->id());
    m_bookDB->getAuthorFromShamela(book->authorID());

    QSqlDatabase::removeDatabase(QString("INDEX_%1").arg(book->id()));
}

void ShamelaIndexer::skipCurrentBook()
{
    m_skipCurrent = true;
}
