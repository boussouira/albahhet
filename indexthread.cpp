#include "indexthread.h"
#include "common.h"
#include "booksdb.h"
#include "indexinfo.h"
#include "bookinfo.h"
#include <qvariant.h>
#include <qsqlquery.h>
#include <qmessagebox.h>

IndexingThread::IndexingThread()
{
    m_stopIndexing = false;
}

void IndexingThread::run()
{
    startIndexing();
}

void IndexingThread::startIndexing()
{
    try {
        BookInfo *book = m_bookDB->next();
        while(book != NULL) {
            emit fileIndexed(book->name());
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

void IndexingThread::indexBook(BookInfo *book)
{
//    qDebug() << " *" << book->name();
    {
        QSqlDatabase mdbDB = QSqlDatabase::addDatabase("QODBC", QString("INDEX_%1").arg(book->id()));
        mdbDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                 .arg(book->path()));

        if (!mdbDB.open()) {
            qDebug("[%s:%d] Cannot open database.", __FILE__, __LINE__);
            return;
        }
        QSqlQuery shaQuery(mdbDB);

        shaQuery.exec(QString("SELECT id, nass FROM %1 ORDER BY id ")
                         .arg((!book->archive()) ? "book" : QString("b%1").arg(book->id())));
        Document doc;

        int tokenAndNoStore = Field::STORE_NO | Field::INDEX_TOKENIZED;
        int storeAndNoToken = Field::STORE_YES | Field::INDEX_UNTOKENIZED;

        while(shaQuery.next())
        {
           doc.add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(shaQuery.value(0).toString()),
                                   storeAndNoToken));
            doc.add( *_CLNEW Field(_T("bookid"), book->idT(), storeAndNoToken));
            doc.add( *_CLNEW Field(_T("archive"), book->archiveT(), storeAndNoToken));
            doc.add( *_CLNEW Field(_T("cat"), book->catT(), storeAndNoToken));
            doc.add( *_CLNEW Field(_T("author"), book->authorIDT(), storeAndNoToken));
            doc.add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(shaQuery.value(1).toString()),
                                   tokenAndNoStore));

            m_writer->addDocument(&doc);

            doc.clear();
        }

        mdbDB.close();
    }

    m_bookDB->setBookIndexed(book->id());
    m_bookDB->getAuthorFromShamela(book->authorID());

    QSqlDatabase::removeDatabase(QString("INDEX_%1").arg(book->id()));
}
