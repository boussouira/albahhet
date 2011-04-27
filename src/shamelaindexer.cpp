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
    m_prevArchive = -1;
    m_indexedBooks.reserve(5000);
    m_authors.reserve(3000);
}

void ShamelaIndexer::run()
{
    m_threadId = (int)currentThreadId();
    qDebug("Starting thread: %d", m_threadId);

    startIndexing();

    qDebug("Update indexed book in thread: %d", m_threadId);
    m_indexedBooks.squeeze();
    m_bookDB->setBookIndexed(m_indexedBooks);

    qDebug("Update authors in thread: %d", m_threadId);
    m_authors.squeeze();
    m_bookDB->getAuthorFromShamela(m_authors);
}

void ShamelaIndexer::startIndexing()
{
    try {
        BookInfo *book = m_bookDB->next();
        while(book != NULL) {
            book->genInfo();

            emit currentBookName(book->name());
            indexBook(book);

            delete book;
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
    QSqlDatabase mdbDB;
    QString connName(QString("_%1_%2").arg(book->archive()).arg(m_threadId));

    if(book->archive() && book->archive() == m_prevArchive) {
        mdbDB = QSqlDatabase::database(connName);
    } else {
        // Remove prevouis connection
        removeDatabase(QString("_%1_%2").arg(m_prevArchive).arg(m_threadId));

        mdbDB = QSqlDatabase::addDatabase("QODBC", connName);
        mdbDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                              .arg(book->path()));
        m_prevArchive = book->archive();
    }

    if(!mdbDB.isOpen()) {
        if (!mdbDB.open()) {
            DB_OPEN_ERROR(book->path());
            return;
        }
    }

    QSqlQuery shaQuery(mdbDB);
    shaQuery.exec(QString("SELECT id, nass FROM %1 ORDER BY id ").arg(book->mainTable()));

    Document doc;
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

//            if(m_skipCurrent) {
//                m_skipCurrent = false;
//                break;
//            }
    }

    m_indexedBooks.insert(book->id());
    m_authors.insert(book->authorID());
}

void ShamelaIndexer::skipCurrentBook()
{
    m_skipCurrent = true;
}

void ShamelaIndexer::removeDatabase(QString &connName)
{
    QSqlDatabase::database(connName, false).close();
    QSqlDatabase::removeDatabase(connName);
}
