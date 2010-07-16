#include "indexthread.h"

IndexingThread::IndexingThread()
{
    m_stopIndexing = false;
}

void IndexingThread::run()
{
    startIndexing();
    QSqlDatabase::removeDatabase("bookIndexThread");
}

void IndexingThread::startIndexing()
{
    try {
        IndexWriter* writer = NULL;
        QDir dir;
        ArabicAnalyzer analyzer;
        if(!dir.exists(INDEX_PATH))
            dir.mkdir(INDEX_PATH);
        if ( IndexReader::indexExists(INDEX_PATH) ){
            if ( IndexReader::isLocked(INDEX_PATH) ){
                printf("Index was locked... unlocking it.\n");
                IndexReader::unlock(INDEX_PATH);
            }

            writer = _CLNEW IndexWriter( INDEX_PATH, &analyzer, true);
        }else{
            writer = _CLNEW IndexWriter( INDEX_PATH ,&analyzer, true);
        }
        writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);

        QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexThread");
        indexDB.setDatabaseName("book_index.db");
        if(!indexDB.open()){
            qDebug("Error opening index db");
            return;
        }
        QSqlQuery *inexQuery = new QSqlQuery(indexDB);

        inexQuery->exec("SELECT shamelaID, bookName, filePath FROM books");

        if(m_ramSize)
            writer->setRAMBufferSizeMB(m_ramSize);
        //if(m_maxDoc)
        //    writer->setMaxBufferedDocs(m_maxDoc);

        while(inexQuery->next() && !m_stopIndexing) {
            indexBook(writer, inexQuery->value(0).toString(), inexQuery->value(2).toString());
            emit fileIndexed(inexQuery->value(1).toString());
        }

        if(m_optimizeIndex)
            writer->optimize();

        writer->close();
        _CLLDELETE(writer);

        delete inexQuery;
        indexDB.close();
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
        emit indexingError();
        terminate();
    }
    QSqlDatabase::removeDatabase("bookIndexThread");
}

void IndexingThread::indexBook(IndexWriter *writer,const QString &bookID, const QString &bookPath)
{
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaIndexBook");
        m_bookDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                 .arg(bookPath));

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open MDB database.";
            return;
        }
        QSqlQuery *m_bookQuery = new QSqlQuery(m_bookDB);

        m_bookQuery->exec("SELECT id, nass FROM book ORDER BY id ");
        Document doc;
        while(m_bookQuery->next())
        {
            doc.clear();
            doc.add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(m_bookQuery->value(0).toString()),
                                   Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
            doc.add( *_CLNEW Field(_T("bookid"), QSTRING_TO_TCHAR(bookID),
                                   Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
            doc.add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(m_bookQuery->value(1).toString()),
                                   Field::STORE_NO | Field::INDEX_TOKENIZED) );

            writer->addDocument(&doc);
        }

        delete m_bookQuery;
        m_bookDB.close();
    }
    QSqlDatabase::removeDatabase("shamelaIndexBook");
}

void IndexingThread::setOptions(bool optimizeIndex, int ramSize, int maxDoc)
{
    m_optimizeIndex = optimizeIndex;
    m_ramSize = ramSize;
    m_maxDoc = maxDoc;
}
