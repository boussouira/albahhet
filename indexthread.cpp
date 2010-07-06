#include "indexthread.h"

IndexBookThread::IndexBookThread()
{
    m_stop = false;
}

IndexBookThread::~IndexBookThread()
{
    qDebug("DESTROCTE ME!");
}

void IndexBookThread::indexCat()
{
    IndexWriter *writer= NULL;
    QDir dir;
    ArabicAnalyzer an;
    QString m_indexPath = randFolderName(2, "tmp_");

    if(!dir.exists(m_indexPath))
        dir.mkdir(m_indexPath);
    if ( IndexReader::indexExists(qPrintable(m_indexPath)) ){
        if ( IndexReader::isLocked(qPrintable(m_indexPath)) ){
            qDebug() << "Index at" << m_indexPath << "was locked... unlocking it.";
            IndexReader::unlock(qPrintable(m_indexPath));
        }

        writer = _CLNEW IndexWriter( qPrintable(m_indexPath), &an, true);
    }else{
        writer = _CLNEW IndexWriter( qPrintable(m_indexPath), &an, true);
    }
    writer->setMaxFieldLength(0x7FFFFFFFL);
    writer->setUseCompoundFile(false);
    if(m_ramFlushSize)
        writer->setRAMBufferSizeMB(m_ramFlushSize);
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
                       inexQuery->value(2).toString(),
                       writer);
        delete inexQuery;
        indexDB.close();
    }
    QSqlDatabase::removeDatabase(m_indexPath);

    writer->setUseCompoundFile(true);
    if(m_optimizeIndex)
        writer->optimize();
    writer->close();
    _CLLDELETE(writer);

    emit doneCatIndexing(m_indexPath, this);
}

void IndexBookThread::indexBoook(const QString &bookID, const QString &bookName, const QString &bookPath,
                                 IndexWriter *pWriter)
{
    {
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
        Document doc;
        while(m_bookQuery->next())
        {
            doc.clear();
            FileDocument(m_bookQuery->value(0).toString(),
                         bookID,
                         m_bookQuery->value(1).toString(),
                         &doc);
            pWriter->addDocument( &doc );
        }
        delete m_bookQuery;
        m_bookDB.close();
    }

    QSqlDatabase::removeDatabase(QString("shamelaIndexBook_%1").arg(bookID));
    emit bookIsIndexed(bookName);
}

void IndexBookThread::FileDocument(const QString &id, const QString &bookid, const QString &text,
                                        Document *doc)
{
    doc->add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(id) ,
                            Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("bookid"), QSTRING_TO_TCHAR(bookid) ,
                            Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(text),
                            Field::STORE_NO | Field::INDEX_TOKENIZED) );
}

void IndexBookThread::run()
{
    try {
        indexCat();
        quit();
    } catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
    }
    finished();
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
