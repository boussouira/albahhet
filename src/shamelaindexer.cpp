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
    m_indexedBooks.reserve(5000);
}

void ShamelaIndexer::run()
{
    m_threadId = (int)currentThreadId();
    qDebug("Starting thread: %d", m_threadId);

    startIndexing();

    qDebug("Update indexed book in thread: %d", m_threadId);
    m_indexedBooks.squeeze();
    m_bookDB->setBookIndexed(m_indexedBooks);
}

void ShamelaIndexer::startIndexing()
{
    try {
        BookInfo *book = m_bookDB->next();
        while(book != 0) {
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
    QString connName(QString("_%1_%2").arg(book->archive()).arg(m_threadId));
    {
        QSqlDatabase mdbDB = QSqlDatabase::addDatabase("QODBC", connName);
        mdbDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                              .arg(book->path()));

        if (!mdbDB.open()) {
            DB_OPEN_ERROR(book->path());
            return;
        }

        QSqlQuery shaQuery(mdbDB);
        shaQuery.exec(QString("SELECT id, nass FROM %1 ORDER BY id ").arg(book->mainTable()));

        Document doc;
        int tokenAndNoStore = Field::STORE_NO | Field::INDEX_TOKENIZED;
        int storeAndNoToken = Field::STORE_YES | Field::INDEX_UNTOKENIZED;

        while(shaQuery.next())
        {
            doc.add( *_CLNEW Field(PAGE_ID_FIELD, QSTRING_TO_TCHAR(shaQuery.value(0).toString()), storeAndNoToken));
            doc.add( *_CLNEW Field(BOOK_ID_FIELD, book->idT(), storeAndNoToken));
            doc.add( *_CLNEW Field(AUTHOR_DEATH_FIELD, book->authorDeathT(), storeAndNoToken));

            QString text = shaQuery.value(1).toString();
            if(!text.contains("__________")) {
                doc.add( *_CLNEW Field(PAGE_TEXT_FIELD, QSTRING_TO_TCHAR(text), tokenAndNoStore));
            } else {
                QStringList texts = text.split("__________", QString::SkipEmptyParts);
                if(texts.count() == 2) {
                    doc.add( *_CLNEW Field(PAGE_TEXT_FIELD, QSTRING_TO_TCHAR(texts.first()), tokenAndNoStore));
                    doc.add( *_CLNEW Field(FOOT_NOTE_FIELD, QSTRING_TO_TCHAR(texts.last()), tokenAndNoStore));
                } else {
                    doc.add( *_CLNEW Field(PAGE_TEXT_FIELD, QSTRING_TO_TCHAR(text), tokenAndNoStore));
                }
            }

            m_writer->addDocument(&doc);
            doc.clear();
        }

        m_indexedBooks.insert(book->id());
    }

    QSqlDatabase::removeDatabase(connName);
}

void ShamelaIndexer::skipCurrentBook()
{
    m_skipCurrent = true;
}
