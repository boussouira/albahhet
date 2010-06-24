#include "indexthread.h"

IndexBookThread::IndexBookThread(IndexWriter *writer)
{
    m_writer = writer;
    m_stop = false;
}

void IndexBookThread::setBook(const QString &bookID, const QString &bookName, const QString &bookPath)
{
    m_bookID = bookID;
    m_bookPath = bookPath;
    m_bookName = bookName;
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
            m_writer->addDocument( FileDocument(m_bookQuery->value(0).toString(),
                                                bookID,
                                                m_bookQuery->value(1).toString()) );
        }
    } catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
    }

    QSqlDatabase::removeDatabase("shamelaIndexBook");
    emit bookIsIndexed(bookName);
    m_gotBookToIndex.wakeAll();
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
    while(!m_stop) {
        m_mutex.lock();
        emit giveNextBook(this);
        m_gotBookToIndex.wait(&m_mutex);
        m_mutex.unlock();
    }
}
void IndexBookThread::stop()
{
    m_stop = true;
}
