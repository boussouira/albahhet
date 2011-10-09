#include "booksdb.h"
#include "common.h"
#include "bookinfo.h"
#include "indexinfo.h"
#include <qvariant.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qstandarditemmodel.h>
#include <QTime>

BooksDB::BooksDB()
{
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

    m_shamelaQuery->exec(QString("SELECT bk, Archive, oVer FROM 0bok WHERE bkid = %1").arg(id));

    if(m_shamelaQuery->next()) {
        book->setId(id);
        book->setArchive(m_shamelaQuery->value(1).toInt());
        book->setName(m_shamelaQuery->value(0).toString());
        book->setBookVersion(m_shamelaQuery->value(2).toInt());
        book->genInfo(m_indexInfo);

        m_bookInfoHash.insert(id, book);
    } else {
        delete book;
        book = 0;
    }

    return book;
}

void BooksDB::clear()
{
    m_indexQuery->finish();
}

void BooksDB::close()
{
    if(m_indexDB.isOpen()) {
        delete m_indexQuery;
        m_indexDB.close();
    }

    if(m_shamelaDB.isOpen()) {
        delete m_shamelaQuery;
        m_shamelaDB.close();
    }

    if(m_shamelaSpecialDB.isOpen()) {
        delete m_shamelaSpecialQuery;
        m_shamelaSpecialDB.close();
    }


    qDeleteAll(m_bookInfoHash);
    m_bookInfoHash.clear();

    qDeleteAll(m_authorsInfo);
    m_authorsInfo.clear();
}

void BooksDB::setBookIndexed(QSet<int> books)
{
    QMutexLocker locker(&m_mutex);

    openIndexDB();
    QSqlQuery query(m_indexDB);

    m_indexDB.transaction();

    foreach(int shaId, books) {
        query.exec(QString("UPDATE books SET indexFLags = 1 WHERE shamelaID = %1").arg(shaId));
    }

    m_indexDB.commit();
}

void BooksDB::setIndexInfo(IndexInfo *info)
{
    close();
    m_indexInfo = info;
}

void BooksDB::openIndexDB()
{
    if(!m_indexDB.isOpen()) {
        QString book = m_indexInfo->indexDbPath();
        QString connName = QString("indexDb_%1").arg(m_indexInfo->id());

        while(QSqlDatabase::contains(connName))
            connName.append("_");

        m_indexDB = QSqlDatabase::addDatabase("QSQLITE", connName);
        m_indexDB.setDatabaseName(book);

        if (!m_indexDB.open()) {
            DB_OPEN_ERROR(book);

            throw tr("لا يمكن فتح قاعدة البيانات الموجودة في المسار:"
                         "\n" "%1").arg(book);
        }

        m_indexQuery = new QSqlQuery(m_indexDB);
    }
}

void BooksDB::openShamelaDB()
{
    if(!m_shamelaDB.isOpen()) {
        QString book = m_indexInfo->shamelaMainDbPath();
        QString connName = QString("shamelaBookDb_%1").arg(m_indexInfo->id());

        while(QSqlDatabase::contains(connName))
            connName.append("_");

        m_shamelaDB = QSqlDatabase::addDatabase("QODBC", connName);
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
        m_shamelaDB.setDatabaseName(mdbpath);

        if (!m_shamelaDB.open()) {
            DB_OPEN_ERROR(book);

            throw tr("لا يمكن فتح قاعدة البيانات الموجودة في المسار:"
                         "\n" "%1").arg(book);
        }

        m_shamelaQuery = new QSqlQuery(m_shamelaDB);
    }
}

void BooksDB::openShamelaSpecialDB()
{
    if(!m_shamelaSpecialDB.isOpen()) {
        QString book = m_indexInfo->shamelaSpecialDbPath();
        QString connName = QString("shamelaSpecialDb_%1").arg(m_indexInfo->id());

        while(QSqlDatabase::contains(connName))
            connName.append("_");

        m_shamelaSpecialDB = QSqlDatabase::addDatabase("QODBC", connName);
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
        m_shamelaSpecialDB.setDatabaseName(mdbpath);

        if (!m_shamelaSpecialDB.open()) {
            DB_OPEN_ERROR(book);

            throw tr("لا يمكن فتح قاعدة البيانات الموجودة في المسار:"
                         "\n" "%1").arg(book);
        }

        m_shamelaSpecialQuery = new QSqlQuery(m_shamelaSpecialDB);
    }
}

void BooksDB::queryBooksToIndex()
{
    openIndexDB();
    m_indexQuery->exec("SELECT shamelaID, bookName, filePath, archive, authorDeath FROM books ORDER BY archive ASC");
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

    m_indexQuery->exec(QString("SELECT shamelaID, bookName, filePath, archive, authorDeath FROM books "
                               "WHERE shamelaID IN(%1)").arg(whereIds));
}

BookInfo *BooksDB::next()
{
    QMutexLocker locker(&m_mutex);

    if(m_indexQuery->next()){
        BookInfo *book = new BookInfo(m_indexQuery->value(0).toInt(),
                                      m_indexQuery->value(1).toString(),
                                      m_indexQuery->value(2).toString(),
                                      m_indexQuery->value(3).toInt());
        book->setAuthorDeath(m_indexQuery->value(4).toInt());

        return book;
    } else {
        return 0;
    }
}

void BooksDB::importBooksListFromShamela()
{
    openIndexDB();
    openShamelaDB();


    m_indexQuery->exec("DROP TABLE IF EXISTS books");
    m_indexQuery->exec("DROP TABLE IF EXISTS fields");
    m_indexQuery->exec("DROP TABLE IF EXISTS fieldsBooks");

    m_indexQuery->exec("CREATE TABLE books (id INTEGER PRIMARY KEY, bookName TEXT, "
                       "shamelaID INTEGER, bookInfo TEXT, filePath TEXT, authorId INTEGER, authorName TEXT, "
                       "authorDeath INTEGER, version INTEGER, cat INTEGER, archive INTEGER, titleTable TEXT, "
                       "bookTable TEXT, indexFLags INTEGER)");

    m_indexQuery->exec("CREATE TABLE fields(id INTEGER PRIMARY KEY, indexID INTEGER, name TEXT)");
    m_indexQuery->exec("CREATE TABLE fieldsBooks(id INTEGER, bookID INTEGER)");

    m_indexDB.transaction();

    m_shamelaQuery->exec("SELECT Bk, bkid, cat, authno, Archive, betaka, oVer FROM 0bok");
    while(m_shamelaQuery->next()) {
        int archive = m_shamelaQuery->value(4).toInt();
        ShamelaAuthorInfo* auth = getAuthorInfo(m_shamelaQuery->value(3).toInt());

        m_indexQuery->prepare("INSERT INTO books "
                              "(bookName, shamelaID, bookInfo, filePath, authorId, authorName, authorDeath, "
                              "version, cat, archive, titleTable, bookTable, indexFLags) "
                              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        m_indexQuery->bindValue(0, m_shamelaQuery->value(0).toString());    // bookName
        m_indexQuery->bindValue(1, m_shamelaQuery->value(1).toInt());       // shamelaID
        m_indexQuery->bindValue(2, m_shamelaQuery->value(5).toString());    // bookInfo
        m_indexQuery->bindValue(3, m_indexInfo->buildFilePath(m_shamelaQuery->value(1).toString(), archive));   // filePath
        m_indexQuery->bindValue(4, auth->authorID);                         // authorId
        m_indexQuery->bindValue(5, auth->name);                             // authorName
        m_indexQuery->bindValue(6, auth->dieYear);                          // authorDeath
        m_indexQuery->bindValue(7, m_shamelaQuery->value(6).toInt());       // version
        m_indexQuery->bindValue(8, m_shamelaQuery->value(2).toInt());       // cat
        m_indexQuery->bindValue(9, archive);                                // archive
        m_indexQuery->bindValue(10, (!archive) ? "title" : QString("t%1").arg(m_shamelaQuery->value(1).toInt()));// titleTable
        m_indexQuery->bindValue(11, (!archive) ? "book" : QString("b%1").arg(m_shamelaQuery->value(1).toInt())); // bookTable
        m_indexQuery->bindValue(12, 0);                                     // indexFLags

        if(!m_indexQuery->exec())
            SQL_ERROR(m_indexQuery->lastError().text());

    }

    m_indexDB.commit();
}

void BooksDB::run()
{
    importBooksListFromShamela();
}

QStringList BooksDB::addBooks(QList<int> shaIds)
{
    openIndexDB();
    openShamelaDB();

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
    QString sql(QString("SELECT Bk, bkid, cat, authno, Archive, betaka, oVer FROM 0bok WHERE bkid IN(%1)")
                .arg(whereIds));

    m_shamelaQuery->exec(sql);


    while(m_shamelaQuery->next()) {
        int archive = m_shamelaQuery->value(4).toInt();
        ShamelaAuthorInfo* auth = getAuthorInfo(m_shamelaQuery->value(3).toInt());

        m_indexQuery->prepare("INSERT INTO books "
                              "(bookName, shamelaID, bookInfo, filePath, authorId, authorName, authorDeath, "
                              "version, cat, archive, titleTable, bookTable, indexFLags) "
                              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        m_indexQuery->bindValue(0, m_shamelaQuery->value(0).toString());    // bookName
        m_indexQuery->bindValue(1, m_shamelaQuery->value(1).toInt());       // shamelaID
        m_indexQuery->bindValue(2, m_shamelaQuery->value(5).toString());    // bookInfo
        m_indexQuery->bindValue(3, m_indexInfo->buildFilePath(m_shamelaQuery->value(1).toString(), archive));   // filePath
        m_indexQuery->bindValue(4, auth->authorID);                         // authorId
        m_indexQuery->bindValue(5, auth->name);                             // authorName
        m_indexQuery->bindValue(6, auth->dieYear);                          // authorDeath
        m_indexQuery->bindValue(7, m_shamelaQuery->value(6).toInt());       // version
        m_indexQuery->bindValue(8, m_shamelaQuery->value(2).toInt());       // cat
        m_indexQuery->bindValue(9, archive);                                // archive
        m_indexQuery->bindValue(10, (!archive) ? "title" : QString("t%1").arg(m_shamelaQuery->value(1).toInt()));// titleTable
        m_indexQuery->bindValue(11, (!archive) ? "book" : QString("b%1").arg(m_shamelaQuery->value(1).toInt())); // bookTable
        m_indexQuery->bindValue(12, 0);                                     // indexFLags

        if(m_indexQuery->exec()) {
            addedBooksName << m_shamelaQuery->value(0).toString();
        } else {
            SQL_ERROR(m_indexQuery->lastError().text());
        }
    }

    m_indexDB.commit();

    return addedBooksName;
}

QStringList BooksDB::removeBooks(QList<int> savedIds)
{
    openIndexDB();
    openShamelaDB();

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
        SQL_ERROR(m_indexQuery->lastError().text());
        return QStringList();
    }
}

QStringList BooksDB::connections()
{
    QStringList list;

    if(m_shamelaDB.isOpen())
        list << m_shamelaDB.connectionName();

    if(m_indexDB.isOpen())
        list << m_indexDB.connectionName();

    if(m_shamelaSpecialDB.isOpen())
        list << m_shamelaSpecialDB.connectionName();

    return list;
}

QStandardItemModel *BooksDB::getSimpleBooksListModel()
{
    openIndexDB();
    QStandardItemModel *model= new QStandardItemModel();

    QSqlQuery query(m_indexDB);

    query.prepare("SELECT shamelaID, bookName, bookInfo, authorName, authorDeath "
                  "FROM books WHERE indexFLags = 1 ");

    if(!query.exec())
        qDebug() << query.lastError().text();

    while(query.next()) {
        QStandardItem *bookItem = new QStandardItem();
        bookItem->setText(query.value(1).toString());
        bookItem->setData(query.value(0).toInt(), idRole);
        bookItem->setData(Book, typeRole);
        bookItem->setToolTip(query.value(2).toString());

        model->appendRow(bookItem);
    }
    return model;
}

QStandardItemModel *BooksDB::getBooksListModel()
{
    openShamelaDB();
    QStandardItemModel *model= new QStandardItemModel();
    QStandardItem *item;

    QTime time;
    time.start();

    if(!m_shamelaQuery->exec("SELECT id, name FROM 0cat ORDER BY catord"))
        qDebug() << m_shamelaQuery->lastError().text();

    while(m_shamelaQuery->next()) {
        item = new QStandardItem();
        item->setText(m_shamelaQuery->value(1).toString());
        item->setData(m_shamelaQuery->value(0).toInt(), idRole);
        item->setData(Categorie, typeRole);
        item->setCheckable(true);

        booksCat(item, m_shamelaQuery->value(0).toInt());

        model->appendRow(item);
    }

    model->setHorizontalHeaderLabels(QStringList()
                                     << tr("الكتاب")
                                     << tr("المؤلف")
                                     << tr("تاريخ الوفاة"));

    qDebug("Load model take %d ms", time.elapsed());
    return model;
}

void BooksDB::booksCat(QStandardItem *parentNode, int catID)
{
    openIndexDB();
    QSqlQuery query(m_indexDB);

    query.prepare("SELECT shamelaID, bookName, bookInfo, authorName, authorDeath "
                  "FROM books WHERE indexFLags = 1 AND cat = ? ");

    query.bindValue(0, catID);
    if(!query.exec())
        qDebug() << query.lastError().text();

    while(query.next()) {
        int row = parentNode->rowCount();
        QStandardItem *bookItem = new QStandardItem();
        bookItem->setText(query.value(1).toString());
        bookItem->setData(query.value(0).toInt(), idRole);
        bookItem->setData(Book, typeRole);
        bookItem->setToolTip(query.value(2).toString());

        bookItem->setCheckable(true);
        bookItem->setCheckState(Qt::Unchecked);

        QStandardItem *authItem = new QStandardItem();
        authItem->setText(query.value(3).toString());

        QStandardItem *authDeathItem = new QStandardItem();
        authDeathItem->setData(query.value(4).toInt(), Qt::DisplayRole);

        parentNode->setChild(row, 0, bookItem);
        parentNode->setChild(row, 1, authItem);
        parentNode->setChild(row, 2, authDeathItem);
    }
}

QList<QPair<QString, QString> > BooksDB::getBookShoorts(int bid)
{
    openShamelaSpecialDB();

    QList<QPair<QString, QString> > shoorts;
    QSqlQuery specialQuery(m_shamelaSpecialDB);

    if(!specialQuery.exec(QString("SELECT Ramz, Nass FROM shorts WHERE Bk = '%1'").arg(bid)))
        SQL_ERROR(specialQuery.lastError().text());

    while(specialQuery.next()) {
        QPair<QString, QString> pair;
        pair.first = specialQuery.value(0).toString();
        pair.second = specialQuery.value(1).toString();

        shoorts.append(pair);
    }

    return shoorts;
}

QList<int> BooksDB::getAuthorBooks(int auth)
{
    openIndexDB();
    QList<int> list;

    if(!m_indexQuery->exec(QString("SELECT shamelaID FROM books WHERE indexFLags = 1 and authorId = %1").arg(auth)))
        SQL_ERROR(m_indexQuery->lastError().text());

    while(m_indexQuery->next())
        list.append(m_indexQuery->value(0).toInt());

    return list;
}

void BooksDB::loadSowarNames()
{
    if(m_sowarNames.isEmpty()) {
        openShamelaSpecialDB();
        m_sowarNames.clear();

        QSqlQuery specialQuery(m_shamelaSpecialDB);

        if(!specialQuery.exec(QString("SELECT sora FROM Sora")))
            SQL_ERROR(specialQuery.lastError().text());

        while(specialQuery.next()) {
            m_sowarNames.append(specialQuery.value(0).toString());
        }
    }
}

QString BooksDB::getSoraName(int soraNumber)
{
    loadSowarNames();

    int num = soraNumber - 1;

    if(0 <= num && num < m_sowarNames.count())
        return m_sowarNames.at(num);
    else
        return "Out of range!";
}

int BooksDB::getAuthorDeath(int authorID)
{
    openShamelaSpecialDB();

    if(authorID > 0) {
        int sdy = m_authorsDeath.value(authorID, 0);

        if(sdy)
            return sdy;

        QSqlQuery specialQuery(m_shamelaSpecialDB);

        specialQuery.prepare("SELECT AD FROM Auth WHERE authid = ?");
        specialQuery.bindValue(0, authorID);

        if(!specialQuery.exec())
            SQL_ERROR(specialQuery.lastError().text());

        if(specialQuery.next()) {
            int dYear = specialQuery.value(0).toInt();
            m_authorsDeath.insert(authorID, dYear);

            return dYear;
        }
    }

    return 99999;
}

ShamelaAuthorInfo* BooksDB::getAuthorInfo(int authorID)
{
    ShamelaAuthorInfo *info = m_authorsInfo.value(authorID, 0);
    if(info)
        return info;

    openShamelaSpecialDB();

    info = new ShamelaAuthorInfo(0, 0, QString());
    QSqlQuery specialQuery(m_shamelaSpecialDB);

    specialQuery.prepare("SELECT auth, AD FROM Auth WHERE authid = ?");
    specialQuery.bindValue(0, authorID);

    if(!specialQuery.exec())
        SQL_ERROR(specialQuery.lastError().text());

    if(specialQuery.next()) {
        info->authorID = authorID;
        info->dieYear = specialQuery.value(1).toInt();
        info->name = specialQuery.value(0).toString();
    }

    m_authorsInfo.insert(authorID, info);

    return info;
}
