#include "booksdb.h"

BooksDB::BooksDB()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "BooksListDB");
    m_db.setDatabaseName("book_index.db");

    if (!m_db.open()) {
        qDebug("[%s:%d] Cannot open database.", __FILE__, __LINE__);
    }

    m_query = new QSqlQuery(m_db);
    m_query->exec("SELECT shamelaID, bookName, filePath, archive FROM books");
}

BookInfo *BooksDB::next()
{
    QMutexLocker locker(&m_mutex);

    return (m_query->next()) ? new BookInfo(m_query->value(0).toString(),
                                            m_query->value(1).toString(),
                                            m_query->value(2).toString(),
                                            m_query->value(3).toString()) : 0;

}

void BooksDB::clear()
{
    m_query->finish();
}
