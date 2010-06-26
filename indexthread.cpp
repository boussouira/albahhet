#include "indexthread.h"

IndexBookThread::IndexBookThread()
{
    m_stop = false;
}

void IndexBookThread::indexCat()
{
    m_writer = NULL;
    QDir dir;
    ArabicAnalyzer *analyzer = new ArabicAnalyzer();
    QString m_indexPath = randFolderName(2, "tmp_");
//    const char *indexPath = qPrintable(m_indexPath);
//    qDebug() << "m_indexPath:" << indexPath;
//    qDebug() << "QPATH:" << m_indexPath;
    if(!dir.exists(m_indexPath))
        dir.mkdir(m_indexPath);
    if ( IndexReader::indexExists(qPrintable(m_indexPath)) ){
        if ( IndexReader::isLocked(qPrintable(m_indexPath)) ){
            qDebug() << "Index at" << m_indexPath << "was locked... unlocking it.";
            IndexReader::unlock(qPrintable(m_indexPath));
        }

        m_writer = _CLNEW IndexWriter( qPrintable(m_indexPath), analyzer, true);
    }else{
        m_writer = _CLNEW IndexWriter( qPrintable(m_indexPath), analyzer, true);
    }
    m_writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
    if(m_ramFlushSize)
        m_writer->setRAMBufferSizeMB(m_ramFlushSize);
    {
        QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", m_indexPath);
        indexDB.setDatabaseName("book_index.db");
        if(!indexDB.open())
            qDebug("Error opning index db");
        QSqlQuery *inexQuery = new QSqlQuery(indexDB);

        inexQuery->exec(QString("SELECT shamelaID, bookName, filePath FROM books "
                                "WHERE cat = %1 ORDER BY fileSize %2")
                        .arg(m_currentCat)
                        .arg(false ? "DESC" : "ASC"));
        while(inexQuery->next() && !m_stop)
            indexBoook(inexQuery->value(0).toString(),
                       inexQuery->value(1).toString(),
                       inexQuery->value(2).toString());
    }
    QSqlDatabase::removeDatabase(m_indexPath);

    if(m_optimizeIndex)
        m_writer->optimize();
    m_writer->close();
    _CLDELETE(m_writer);
    emit doneCatIndexing(m_indexPath);
    /*
    try {
        indexBoook();
    } catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
    }*/
}

void IndexBookThread::indexBoook(const QString &bookID, const QString &bookName, const QString &bookPath)
{
    try {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC",
                                                          QString("shamelaIndexBook_%1").arg(bookID));
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(bookPath);
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open" << bookPath << "database.";
        }
        QSqlQuery *m_bookQuery = new QSqlQuery(m_bookDB);

        m_bookQuery->exec("SELECT id, nass FROM book ORDER BY id ");
        while(m_bookQuery->next())
        {
            m_writer->addDocument( FileDocument(m_bookQuery->value(0).toString(),
                                                bookID,
                                                m_bookQuery->value(1).toString()) );
        }
    } catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
    }

    QSqlDatabase::removeDatabase(QString("shamelaIndexBook_%1").arg(bookID));
    emit bookIsIndexed(bookName);
}

Document* IndexBookThread::FileDocument(const QString &id, const QString &bookid, const QString &text)
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

void IndexBookThread::run()
{
    indexCat();
}

void IndexBookThread::stop()
{
    m_stop = true;
}

QString IndexBookThread::randFolderName(int len, const QString &prefix)
{
    static int c = QTime::currentTime().msec();
    QString str = "0123456789abcdef";
    QString folder;
    QDir dir;
    do {
        folder.clear();
        qsrand(++c);
        for(int i=0 ; i<len;i++)
            folder.append(str[(int)qrand() % 15]);
    } while(dir.exists(folder));

    if(prefix.isEmpty())
        return folder;
    else
        return prefix + folder;
}
