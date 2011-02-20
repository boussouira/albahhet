#include "booksdb.h"
#include "bookinfo.h"
#include "indexinfo.h"
#include <qvariant.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qstandarditemmodel.h>

BooksDB::BooksDB()
{
    m_indexDbIsOpen = false;
    m_shamelaDbIsOpen = false;
    m_shamelaSpecialDbIsOpen = false;
}

BooksDB::~BooksDB()
{
    close();
}

BookInfo *BooksDB::getBookInfo(int id)
{
    openShamelaDB();
    BookInfo *book = m_bookInfoHash.value(id, 0);

    if(book)
        return book;

    book = new BookInfo();

    m_shamelaQuery->exec(QString("SELECT bk, Archive FROM 0bok WHERE bkid = %1").arg(id));

    if(m_shamelaQuery->next()) {
        book->setId(id);
        book->setArchive(m_shamelaQuery->value(1).toInt());
        book->setName(m_shamelaQuery->value(0).toString());
        book->genInfo(m_indexInfo);

        m_bookInfoHash.insert(id, book);
    }

    return book;
}

BookInfo *BooksDB::next()
{
    QMutexLocker locker(&m_mutex);

    if(m_indexQuery->next()){
        BookInfo *book = new BookInfo(m_indexQuery->value(0).toInt(),
                                      m_indexQuery->value(1).toString(),
                                      m_indexQuery->value(2).toString(),
                                      m_indexQuery->value(3).toInt());
        book->setCat(m_indexQuery->value(4).toInt());
        book->setAuthorID(m_indexQuery->value(5).toInt());

        return book;
    } else {
        return NULL;
    }
}

void BooksDB::clear()
{
    m_indexQuery->finish();
}

void BooksDB::close()
{
    if(m_indexDbIsOpen) {
        delete m_indexQuery;

        m_indexDB.close();
        m_indexDbIsOpen = false;
    }

    if(m_shamelaDbIsOpen) {
        delete m_shamelaQuery;

        m_shamelaDB.close();
        m_shamelaDbIsOpen = false;
    }

    if(m_shamelaSpecialDbIsOpen) {
        delete m_shamelaSpecialQuery;

        m_shamelaSpecialDB.close();
        m_shamelaSpecialDbIsOpen = false;
    }

    if(!m_bookInfoHash.isEmpty()) {
        qDeleteAll(m_bookInfoHash);
        m_bookInfoHash.clear();
    }
}

void BooksDB::setBookIndexed(int shaId)
{
    openIndexDB();
    QSqlQuery query(m_indexDB);

    query.exec(QString("UPDATE books SET indexFLags = 1 WHERE shamelaID = %1").arg(shaId));
}
void BooksDB::getAuthorFromShamela(int id)
{
    if(!id || m_savedAuthors.contains(id))
        return;

    openIndexDB();
    openShamelaSpecialDB();

    QSqlQuery query(m_indexDB);
    QSqlQuery shamelaQuery(m_shamelaSpecialDB);

    query.exec(QString("SELECT COUNT(id) FROM authors WHERE shamelaAuthorID = %1").arg(id));
    if(query.next()) {
        if(query.value(0).toInt() == 0) {
            shamelaQuery.exec(QString("SELECT authid, auth FROM Auth WHERE authid = %1").arg(id));
            if(shamelaQuery.next()) {
                query.exec(QString("INSERT INTO authors VALUES (NULL, %1, '%2')")
                           .arg(id)
                           .arg(shamelaQuery.value(1).toString()));
                m_savedAuthors.append(id);
            }
        }
    }
}

void BooksDB::setIndexInfo(IndexInfo *info)
{
    close();

    m_indexInfo = info;
    QString hash = m_indexInfo->indexHash();
    m_indexConnName = "indexDb_" + hash;
    m_shaConnName = "shamelaBookDb_" + hash;
    m_shaSpeConnName = "shamelaSpecialDb_" + hash;
}

void BooksDB::openIndexDB()
{
    if(m_indexDbIsOpen)
        return;

    QString book = m_indexInfo->indexDbPath();

    m_indexDB = QSqlDatabase::addDatabase("QSQLITE", m_indexConnName);
    m_indexDB.setDatabaseName(book);

    if (!m_indexDB.open()) {
        qDebug("[%s:%d] Cannot open database at \"%s\".", __FILE__, __LINE__, qPrintable(book));
        return;
    }

    m_indexQuery = new QSqlQuery(m_indexDB);
    m_indexDbIsOpen = true;
}

void BooksDB::openShamelaDB()
{
    if(m_shamelaDbIsOpen)
        return;

    QString book = m_indexInfo->shamelaMainDbPath();

    m_shamelaDB = QSqlDatabase::addDatabase("QODBC", m_shaConnName);
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
    m_shamelaDB.setDatabaseName(mdbpath);

    if (!m_shamelaDB.open()) {
        qDebug("[%s:%d] Cannot open database at \"%s\".", __FILE__, __LINE__, qPrintable(book));
        return;
    }

    m_shamelaQuery = new QSqlQuery(m_shamelaDB);
    m_shamelaDbIsOpen = true;
}

void BooksDB::openShamelaSpecialDB()
{
    if(m_shamelaSpecialDbIsOpen)
        return;

    QString book = m_indexInfo->shamelaSpecialDbPath();

    m_shamelaSpecialDB = QSqlDatabase::addDatabase("QODBC", m_shaSpeConnName);
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
    m_shamelaSpecialDB.setDatabaseName(mdbpath);

    if (!m_shamelaSpecialDB.open()) {
        qDebug("[%s:%d] Cannot open database at \"%s\".", __FILE__, __LINE__, qPrintable(book));
        return;
    }

    m_shamelaSpecialQuery = new QSqlQuery(m_shamelaDB);
    m_shamelaSpecialDbIsOpen = true;
}

void BooksDB::queryBooksToIndex()
{
    openIndexDB();
    m_indexQuery->exec("SELECT shamelaID, bookName, filePath, archive, cat, authorId FROM books");
}

void BooksDB::queryBooksToIndex(QList<int> ids)
{
    openIndexDB();
    QString whereIds;

    for(int i=0; i<ids.count(); i++) {
        if(i>0)
            whereIds.append(",");
        whereIds.append(QString::number(ids.at(i)));
    }

    m_indexQuery->exec(QString("SELECT shamelaID, bookName, filePath, archive, cat, authorId FROM books "
                               "WHERE shamelaID IN(%1)").arg(whereIds));
}

void BooksDB::importBooksListFromShamela()
{
    openIndexDB();
    openShamelaDB();


    m_indexQuery->exec("DROP TABLE IF EXISTS books");
    m_indexQuery->exec("CREATE TABLE IF NOT EXISTS books (id INTEGER PRIMARY KEY, bookName TEXT, "
                    "shamelaID INTEGER, filePath TEXT, authorId INTEGER, authorName TEXT, "
                    "fileSize INTEGER, cat INTEGER, archive INTEGER, titleTable TEXT, bookTable TEXT, "
                    "indexFLags INTEGER)");

    m_indexQuery->exec("DROP TABLE IF EXISTS authors");
    m_indexQuery->exec("CREATE TABLE IF NOT EXISTS authors (id INTEGER PRIMARY KEY, "
                       "shamelaAuthorID INTEGER, name TEXT)");


    m_indexDB.transaction();

    m_shamelaQuery->exec(QString("SELECT Bk, bkid, auth, authno, Archive, cat FROM 0bok"));
    while(m_shamelaQuery->next()) {
        int archive = m_shamelaQuery->value(4).toInt();
        QString sql = QString("INSERT INTO books VALUES "
                              "(NULL, '%1', %2, '%3', %4, '%5', '', %9, %6, '%7', '%8', 0)")
                .arg(m_shamelaQuery->value(0).toString())
                .arg(m_shamelaQuery->value(1).toString())
                .arg(m_indexInfo->buildFilePath(m_shamelaQuery->value(1).toString(), archive))
                .arg(m_shamelaQuery->value(3).toString())
                .arg(m_shamelaQuery->value(2).toString())
                .arg(archive)
                .arg((!archive) ? "title" : QString("t%1").arg(m_shamelaQuery->value(1).toInt()))
                .arg((!archive) ? "book" : QString("b%1").arg(m_shamelaQuery->value(1).toInt()))
                .arg(m_shamelaQuery->value(5).toInt());

        if(!m_indexQuery->exec(sql))
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

QStringList BooksDB::addBooks(QList<int> shaIds)
{
    QStringList addedBooksName;
    QString whereIds;

    if(shaIds.count() <= 0)
        return addedBooksName;

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
                      "(NULL, '%1', %2, '%3', %4, '%5', '', '', %6, '%7', '%8', 0)")
                .arg(m_shamelaQuery->value(0).toString())
                .arg(m_shamelaQuery->value(1).toString())
                .arg(m_indexInfo->buildFilePath(m_shamelaQuery->value(1).toString(), archive))
                .arg(m_shamelaQuery->value(3).toString())
                .arg(m_shamelaQuery->value(2).toString())
                .arg(archive)
                .arg((!archive) ? "title" : QString("t%1").arg(m_shamelaQuery->value(1).toInt()))
                .arg((!archive) ? "book" : QString("b%1").arg(m_shamelaQuery->value(1).toInt()));

        if(m_indexQuery->exec(sql)) {
            addedBooksName << m_shamelaQuery->value(0).toString();
        } else {
            qDebug()<< "ERROR:" << m_indexQuery->lastError().text();
        }
    }

    m_indexDB.commit();

    return addedBooksName;
}

QStringList BooksDB::removeBooks(QList<int> savedIds)
{
    QStringList removedBooks;
    if(savedIds.count() <= 0)
        return removedBooks;

    QString whereIds;
    for(int i=0; i<savedIds.count(); i++) {
        if(i>0)
            whereIds.append(",");
        whereIds.append(QString::number(savedIds.at(i)));
    }

    QString selectSql(QString("SELECT bookName FROM books WHERE shamelaID IN(%1)").arg(whereIds));

    if(m_indexQuery->exec(selectSql)) {
        while(m_indexQuery->next()) {
            removedBooks << m_indexQuery->value(0).toString();
        }
    }


    QString deleteSql(QString("DELETE FROM books WHERE shamelaID IN(%1)").arg(whereIds));

    if(m_indexQuery->exec(deleteSql)) {
        return removedBooks;
    } else {
        qDebug() << "ERROR:" << m_indexQuery->lastError().text();
        return QStringList();
    }
}

QStringList BooksDB::connections()
{
    QStringList list;

    if(m_shamelaDbIsOpen)
        list << m_shaConnName;

    if(m_indexDbIsOpen)
        list << m_indexConnName ;

    if(m_shamelaSpecialDbIsOpen)
        list << m_shaSpeConnName;

    return list;
}

QStandardItemModel *BooksDB::getBooksListModel()
{
    openIndexDB();
    QStandardItemModel *model= new QStandardItemModel();
    QStandardItem *item;

    m_indexQuery->exec("SELECT shamelaID, bookName FROM books WHERE indexFLags = 1");

    while(m_indexQuery->next()) {
        item = new QStandardItem(m_indexQuery->value(1).toString());
        item->setData(m_indexQuery->value(0).toInt(), Qt::UserRole);
        item->setCheckable(true);
        model->appendRow(item);
    }

    return model;
}

QStandardItemModel *BooksDB::getCatsListModel()
{
    openShamelaDB();
    QStandardItemModel *model= new QStandardItemModel();
    QStandardItem *item;

    m_shamelaQuery->exec("SELECT id, name FROM 0cat ORDER BY catord");

    while(m_shamelaQuery->next()) {
        item = new QStandardItem(m_shamelaQuery->value(1).toString());
        item->setData(m_shamelaQuery->value(0).toInt(), Qt::UserRole);
        item->setCheckable(true);
        model->appendRow(item);
    }

    return model;
}

QStandardItemModel *BooksDB::getAuthorsListModel()
{
    openIndexDB();
    QStandardItemModel *model= new QStandardItemModel();
    QStandardItem *item;

    item = new QStandardItem(trUtf8("-- غير محدد --"));
    item->setData(0);
    item->setCheckable(true);
    model->appendRow(item);

    m_indexQuery->exec("SELECT shamelaAuthorID, name FROM authors");

    while(m_indexQuery->next()) {
        item = new QStandardItem(m_indexQuery->value(1).toString());
        item->setData(m_indexQuery->value(0).toInt(), Qt::UserRole);
        item->setCheckable(true);
        model->appendRow(item);
    }

    return model;
}

