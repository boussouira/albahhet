#include "indexthread.h"

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
		writer->setInfoStream(&std::cout);
        writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);

        QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexThread");
        indexDB.setDatabaseName("book_index.db");
        if(!indexDB.open())
            qDebug("Error opning index db");
        QSqlQuery *inexQuery = new QSqlQuery(indexDB);

        inexQuery->exec("SELECT shamelaID, bookName, filePath FROM books");

        if(m_ramSize)
            writer->setRAMBufferSizeMB(m_ramSize);
        if(m_maxDoc)
            writer->setMaxBufferedDocs(m_maxDoc);
        qDebug() << "AUTO FLUSH:" << writer->DISABLE_AUTO_FLUSH;
        qDebug() << "MAX DOC:" << writer->getMaxMergeDocs();
        qDebug() << "RAM SIZE:" << writer->getRAMBufferSizeMB();

        while(inexQuery->next()) {
            if(m_stopIndexing)
                break;

            indexBook(writer, inexQuery->value(0).toString(), inexQuery->value(2).toString());
            emit fileIndexed(inexQuery->value(1).toString());
        }

        if(m_optimizeIndex)
            writer->optimize();

        writer->close();
        _CLDELETE(writer);
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing", err.what());
    }
}

void IndexingThread::indexBook(IndexWriter *writer,const QString &bookID, const QString &bookPath)
{
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaIndexBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(bookPath);
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

    doc->add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(id) ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("bookid"), QSTRING_TO_TCHAR(bookid) ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(text), Field::STORE_NO | Field::INDEX_TOKENIZED) );

    return doc;
}

void IndexingThread::setOptions(bool optimizeIndex, int ramSize, int maxDoc)
{
    m_optimizeIndex = optimizeIndex;
    m_ramSize = ramSize;
    m_maxDoc = maxDoc;
}
