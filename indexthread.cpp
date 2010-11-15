#include "indexthread.h"

IndexingThread::IndexingThread()
{
    m_stopIndexing = false;
}

IndexingThread::IndexingThread(BooksDB *bookDB): m_bookDB(bookDB)
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
        while(book != 0) {
            indexBook(book->id(), book->path(), book->arhive());
            emit fileIndexed(book->name());
//            qDebug() << "FILE:" << book->id() << "THREAD:" << currentThreadId();
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
    catch(exception &err){
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

void IndexingThread::indexBook(const QString &bookID, const QString &bookPath, const QString &archive)
{
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", QString("INDEX_%1").arg(bookID));
        m_bookDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                 .arg(bookPath));

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open MDB database.";
            return;
        }
        QSqlQuery m_bookQuery(m_bookDB);

        m_bookQuery.exec(QString("SELECT id, nass FROM %1 ORDER BY id ")
                         .arg((!archive.toInt()) ? "book" : QString("b%1").arg(archive)));
        Document doc;
        while(m_bookQuery.next())
        {
            doc.clear();
            doc.add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(m_bookQuery.value(0).toString()),
                                   Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
            doc.add( *_CLNEW Field(_T("bookid"), QSTRING_TO_TCHAR(bookID),
                                   Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
            doc.add( *_CLNEW Field(_T("archive"), QSTRING_TO_TCHAR(archive),
                                   Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
            doc.add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(m_bookQuery.value(1).toString()),
                                   Field::STORE_NO | Field::INDEX_TOKENIZED) );

            m_writer->addDocument(&doc);
        }

        m_bookDB.close();
    }
    QSqlDatabase::removeDatabase(QString("INDEX_%1").arg(bookID));
}
