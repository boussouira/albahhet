#include "indexthread.h"

IndexingThread::IndexingThread()
{
    m_stopIndexing = false;
    m_threadCount = 1;
    m_sem = new QSemaphore(0);
}

void IndexingThread::run()
{
    writer = NULL;
    QDir dir;
    ArabicAnalyzer analyzer;
    if(!dir.exists(INDEX_PATH))
        dir.mkdir(INDEX_PATH);
    if ( IndexReader::indexExists(INDEX_PATH) ){
        if ( IndexReader::isLocked(INDEX_PATH) ){
            qDebug() << "Index was locked... unlocking it.";
            IndexReader::unlock(INDEX_PATH);
        }

        writer = _CLNEW IndexWriter( INDEX_PATH, &analyzer, true);
    }else{
        writer = _CLNEW IndexWriter( INDEX_PATH ,&analyzer, true);
    }
    writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);

    indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexThread");
    indexDB.setDatabaseName("book_index.db");
    if(!indexDB.open())
        qDebug("Error opning index db");
    inexQuery = new QSqlQuery(indexDB);

    startIndexing();
    m_sem->acquire(m_threadCount); // wait child threads
    finishIndexing();
}

void IndexingThread::startIndexing()
{
    inexQuery->exec("SELECT shamelaID, bookName, filePath FROM books ORDER BY fileSize");

    if(m_ramSize)
        writer->setRAMBufferSizeMB(m_ramSize);
    if(m_maxDoc)
        writer->setMaxBufferedDocs(m_maxDoc);

    for(int i=0; i<m_threadCount;i++){
        IndexBookThread *book = new IndexBookThread(writer);
        connect(book, SIGNAL(giveNextBook(IndexBookThread*)), this, SLOT(nextBook(IndexBookThread*)));
        connect(book, SIGNAL(bookIsIndexed(QString)), this, SLOT(threadFinishBook(QString)));
        book->start();
    }
}

void IndexingThread::threadFinishBook(const QString &book)
{
    emit fileIndexed(book);
}
void IndexingThread::finishIndexing()
{
    if(m_optimizeIndex)
        writer->optimize();
    delete inexQuery;
    indexDB.close();
    QSqlDatabase::removeDatabase("bookIndexThread");
    writer->close();
    _CLDELETE(writer);
}

void IndexingThread::indexBook(IndexWriter *writer,const QString &bookID, const QString &bookPath)
{
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaIndexBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(bookPath);
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open MDB database.";
        }
        QSqlQuery *m_bookQuery = new QSqlQuery(m_bookDB);

        m_bookQuery->exec("SELECT id, nass FROM book ORDER BY id ");
        while(m_bookQuery->next())
        {
            writer->addDocument( FileDocument(m_bookQuery->value(0).toString(),
                                              bookID,
                                              m_bookQuery->value(1).toString()) );
        }
    }
    QSqlDatabase::removeDatabase("shamelaIndexBook");
}

Document* IndexingThread::FileDocument(const QString &id, const QString &bookid, const QString &text)
{
    // make a new, empty document
    Document* doc = _CLNEW Document();

    doc->add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(id) ,
                            Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("bookid"), QSTRING_TO_TCHAR(bookid) ,
                            Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(text),
                            Field::STORE_NO | Field::INDEX_TOKENIZED) );

    return doc;
}

void IndexingThread::setOptions(bool optimizeIndex, int ramSize, int maxDoc, int threadsCount)
{
    m_optimizeIndex = optimizeIndex;
    m_ramSize = ramSize;
    m_maxDoc = maxDoc;
    m_threadCount = threadsCount;
}

void IndexingThread::nextBook(IndexBookThread *thread)
{
    m_mutex.lock();
    if(inexQuery->next() && !m_stopIndexing) {
        thread->setBook(inexQuery->value(0).toString(),
                        inexQuery->value(1).toString(),
                        inexQuery->value(2).toString());
    } else {
        thread->stop();
        m_sem->release();
    }
    m_mutex.unlock();
}

// IndexBookThread class

IndexBookThread::IndexBookThread(IndexWriter *writer)
{
    m_writer = writer;
    m_indexing = false;
    m_stop = false;
}

void IndexBookThread::setBook(const QString &bookID, const QString &bookName, const QString &bookPath)
{
    m_bookID = bookID;
    m_bookPath = bookPath;
    m_bookName = bookName;
    if(!m_indexing) {
        try {
            indexBoook();
        } catch(CLuceneError &err) {
            QMessageBox::warning(0, "Error when Indexing",
                                 tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
        }
    }
}

void IndexBookThread::indexBoook()
{
    m_indexing = true;
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaIndexBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(m_bookPath);
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open MDB database.";
        }
        QSqlQuery *m_bookQuery = new QSqlQuery(m_bookDB);

        m_bookQuery->exec("SELECT id, nass FROM book ORDER BY id ");
        while(m_bookQuery->next())
        {
            m_writer->addDocument( IndexingThread::FileDocument(m_bookQuery->value(0).toString(),
                                              m_bookID,
                                              m_bookQuery->value(1).toString()) );
        }
    }
    QSqlDatabase::removeDatabase("shamelaIndexBook");
    emit bookIsIndexed(m_bookName);
    m_indexing = false;
    run();
}

void IndexBookThread::run()
{
    if(!m_indexing && !m_stop)
        emit giveNextBook(this);
}
void IndexBookThread::stop()
{
    m_stop = true;
}
