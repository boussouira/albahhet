#include "booksdb.h"

BooksDB::BooksDB()
{
    m_indexDbIsOpen = false;
    m_shamelaDbIsOpen = false;
}

BooksDB::~BooksDB()
{
    if(m_indexDbIsOpen) {
        delete m_indexQuery;
        m_indexDB.close();
    }

    if(m_shamelaDbIsOpen) {
        delete m_shamelaQuery;
        m_shamelaDB.close();
    }
}

BookInfo *BooksDB::next()
{
    QMutexLocker locker(&m_mutex);

    return (m_indexQuery->next()) ? new BookInfo(m_indexQuery->value(0).toString(),
                                            m_indexQuery->value(1).toString(),
                                            m_indexQuery->value(2).toString(),
                                            m_indexQuery->value(3).toString()) : NULL;

}

void BooksDB::clear()
{
    m_indexQuery->finish();
}

void BooksDB::openIndexDB()
{
    if(m_indexDbIsOpen)
        return;

    QString book = m_indexInfo->indexDbPath();

    m_indexDB = QSqlDatabase::addDatabase("QSQLITE", "indexDb");
    m_indexDB.setDatabaseName(book);

    if (!m_indexDB.open()) {
        qDebug("[%s:%d] Cannot open database.", __FILE__, __LINE__);
        return;
    }

    m_indexQuery = new QSqlQuery(m_indexDB);
    m_indexDbIsOpen = true;
}

void BooksDB::openShamelaDB()
{
    if(m_shamelaDbIsOpen)
        return;

    QString book = m_indexInfo->shamelaDbPath();

    m_shamelaDB = QSqlDatabase::addDatabase("QODBC", "shamelaBookDb");
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
    m_shamelaDB.setDatabaseName(mdbpath);

    if (!m_shamelaDB.open()) {
        qDebug("[%s:%d] Cannot open database.", __FILE__, __LINE__);
        return;
    }

    m_shamelaQuery = new QSqlQuery(m_shamelaDB);
    m_shamelaDbIsOpen = true;
}

void BooksDB::queryBooksToIndex()
{
    m_indexQuery->exec("SELECT shamelaID, bookName, filePath, archive FROM books");
}

void BooksDB::queryBooksToIndex(QList<int> ids)
{
    QString whereIds;

    for(int i=0; i<ids.count(); i++) {
        if(i>0)
            whereIds.append(",");
        whereIds.append(QString::number(ids.at(i)));
    }

    m_indexQuery->exec(QString("SELECT shamelaID, bookName, filePath, archive FROM books "
                               "WHERE shamelaID IN(%1)").arg(whereIds));
}

void BooksDB::importBooksListFromShamela()
{
    openIndexDB();
    openShamelaDB();


    m_indexQuery->exec("DROP TABLE IF EXISTS books");
    m_indexQuery->exec("CREATE TABLE IF NOT EXISTS books (id INTEGER PRIMARY KEY, bookName TEXT, "
                    "shamelaID INTEGER, filePath TEXT, authorId INTEGER, authorName TEXT, "
                    "fileSize INTEGER, cat INTEGER, archive INTEGER, titleTable TEXT, bookTable TEXT)");


    m_indexDB.transaction();

    m_shamelaQuery->exec(QString("SELECT Bk, bkid, auth, authno, Archive FROM 0bok"));
        while(m_shamelaQuery->next()) {
        int archive = m_shamelaQuery->value(4).toInt();
        if(!m_indexQuery->exec(QString("INSERT INTO books VALUES "
                                    "(NULL, '%1', %2, '%3', %4, '%5', '', '', %6, '%7', '%8')")
            .arg(m_shamelaQuery->value(0).toString())
            .arg(m_shamelaQuery->value(1).toString())
            .arg(m_indexInfo->buildFilePath(m_shamelaQuery->value(1).toString(), archive))
            .arg(m_shamelaQuery->value(3).toString())
            .arg(m_shamelaQuery->value(2).toString())
            .arg(archive)
            .arg((!archive) ? "title" : QString("t%1").arg(m_shamelaQuery->value(1).toInt()))
            .arg((!archive) ? "book" : QString("b%1").arg(m_shamelaQuery->value(1).toInt()))))
            qDebug()<< "ERROR:" << m_indexQuery->lastError().text();
    }

    m_indexDB.commit();
}

void BooksDB::run()
{
    importBooksListFromShamela();
}


QList<int> BooksDB::getShamelaIds()
{
    openShamelaDB();

    QList<int> shamelaIds;
    m_shamelaQuery->exec("SELECT bkid FROM 0bok ORDER BY bkid");

    while(m_shamelaQuery->next())
        shamelaIds.append(m_shamelaQuery->value(0).toInt());

    return shamelaIds;
}

QList<int> BooksDB::getSavedIds()
{
    openIndexDB();

    QList<int> savedIds;
    m_indexQuery->exec("SELECT shamelaID FROM books ORDER BY id");

    while(m_indexQuery->next())
        savedIds.append(m_indexQuery->value(0).toInt());

    return savedIds;
}

int BooksDB::addBooks(QList<int> shaIds)
{
    if(shaIds.count() <= 0)
        return 0;

    QString whereIds;
    int addedBooks = 0;

    for(int i=0; i<shaIds.count(); i++) {
        if(i>0)
            whereIds.append(",");
        whereIds.append(QString::number(shaIds.at(i)));
    }

    m_indexDB.transaction();
    QString sql(QString("SELECT Bk, bkid, auth, authno, Archive FROM 0bok WHERE bkid IN(%1)")
                .arg(whereIds));

    m_shamelaQuery->exec(sql);
    while(m_shamelaQuery->next()) {
        int archive = m_shamelaQuery->value(4).toInt();
        sql = QString("INSERT INTO books VALUES "
                      "(NULL, '%1', %2, '%3', %4, '%5', '', '', %6, '%7', '%8')")
                .arg(m_shamelaQuery->value(0).toString())
                .arg(m_shamelaQuery->value(1).toString())
                .arg(m_indexInfo->buildFilePath(m_shamelaQuery->value(1).toString(), archive))
                .arg(m_shamelaQuery->value(3).toString())
                .arg(m_shamelaQuery->value(2).toString())
                .arg(archive)
                .arg((!archive) ? "title" : QString("t%1").arg(m_shamelaQuery->value(1).toInt()))
                .arg((!archive) ? "book" : QString("b%1").arg(m_shamelaQuery->value(1).toInt()));

        if(m_indexQuery->exec(sql)) {
            addedBooks++;
        } else {
            qDebug()<< "ERROR:" << m_indexQuery->lastError().text();
        }
    }

    if(m_indexDB.commit())
        return addedBooks;
    else
        return -1;
}

int BooksDB::removeBooks(QList<int> savedIds)
{
    if(savedIds.count() <= 0)
        return 0;

    QString whereIds;
    for(int i=0; i<savedIds.count(); i++) {
        if(i>0)
            whereIds.append(",");
        whereIds.append(QString::number(savedIds.at(i)));
    }

    QString sql(QString("DELETE FROM books WHERE shamelaID IN(%1)").arg(whereIds));

    if(m_indexQuery->exec(sql)) {
        return savedIds.count();
    } else {
        qDebug()<< "ERROR:" << m_indexQuery->lastError().text();
        return -1;
    }
}
