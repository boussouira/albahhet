#include "shamelaupdater.h"
#include "common.h"

#include <QVariant>
#include <QSet>
#include <QIcon>

uint qHash(ShamelaUpdaterTask t) {
    return t.bookID | (t.bookVersion << 21) ;
}

bool ShamelaUpdaterTask::operator ==(const ShamelaUpdaterTask &s) const
{
    return (s.bookID == bookID &&
            s.bookVersion == bookVersion);
}

QString ShamelaUpdaterTask::toString()
{
    return QString("%1;%2;%3;%4")
            .arg(bookID)
            .arg(bookVersion)
            .arg(task)
            .arg(bookName);
}

void ShamelaUpdaterTask::fromString(QString text)
{
    QStringList list = text.split(';', QString::SkipEmptyParts);

    if(list.count() == 4) {
        bookID = list.at(0).toInt();
        bookVersion = list.at(1).toInt();
        task = (Task)list.at(2).toInt();
        bookName = list.at(3);
    } else {
        qDebug("List doesn't contains four argument");
    }
}

ShamelaUpdater::ShamelaUpdater()
{
    m_query = 0;
    m_shamelaQuery = 0;
}

ShamelaUpdater::~ShamelaUpdater()
{
    close();
}

void ShamelaUpdater::setIndexInfo(ShamelaIndexInfo *index)
{
    m_indexInfo = index;

    close();
    openDB();
}

void ShamelaUpdater::close()
{
    if(m_indexDB.isOpen()) {
        if(m_query) {
            delete m_query;
            m_query = 0;
        }

        m_indexDB.close();
        m_indexDB = QSqlDatabase();
        QSqlDatabase::removeDatabase("ShamelaUpdater_index");
    }

    if(m_shamelaDB.isOpen()) {
        if(m_shamelaQuery) {
            delete m_shamelaQuery;
            m_shamelaQuery = 0;
        }

        m_shamelaDB.close();
        m_shamelaDB = QSqlDatabase();
        QSqlDatabase::removeDatabase("ShamelaUpdater_shamela");
    }
}

void ShamelaUpdater::openDB()
{
    if(!m_indexDB.isOpen()) {
        connName = QString("SearchField_%1").arg(m_indexInfo->id());
        QString dbPath = m_indexInfo->indexDbPath();

        m_indexDB = QSqlDatabase::addDatabase("QSQLITE", "ShamelaUpdater_index");
        m_indexDB.setDatabaseName(dbPath);

        if (!m_indexDB.open()) {
            DB_OPEN_ERROR(dbPath);
        }

        m_query = new QSqlQuery(m_indexDB);
    }

    if(!m_shamelaDB.isOpen()) {
        QString book = m_indexInfo->shamelaMainDbPath();

        m_shamelaDB = QSqlDatabase::addDatabase("QODBC", "ShamelaUpdater_shamela");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                .arg(book);
        m_shamelaDB.setDatabaseName(mdbpath);

        if (!m_shamelaDB.open()) {
            DB_OPEN_ERROR(book);
        }

        m_shamelaQuery = new QSqlQuery(m_shamelaDB);
    }
}

void ShamelaUpdater::loadBooks()
{
    m_tasks.clear();
    QList<ShamelaUpdaterTask> shaBooks;

    m_shamelaQuery->exec("SELECT bkid, bk, oVer FROM 0bok ORDER BY bkid");
    while(m_shamelaQuery->next()) {
        ShamelaUpdaterTask task;
        task.bookID = m_shamelaQuery->value(0).toInt();
        task.bookName = m_shamelaQuery->value(1).toString();
        task.bookVersion = m_shamelaQuery->value(2).toInt();
        task.task = ShamelaUpdaterTask::Add;

        shaBooks.append(task);
    }


    QList<ShamelaUpdaterTask> libBooks;
    m_query->exec("SELECT shamelaID, bookName, version FROM books ORDER BY shamelaID");
    while(m_query->next()) {
        ShamelaUpdaterTask task;
        task.bookID = m_query->value(0).toInt();
        task.bookName = m_query->value(1).toString();
        task.bookVersion = m_query->value(2).toInt();
        task.task = ShamelaUpdaterTask::Delete;

        libBooks.append(task);
    }

    foreach (ShamelaUpdaterTask book, shaBooks) {
        if(libBooks.contains(book)) {
            libBooks.removeAll(book);
            shaBooks.removeAll(book);
        }
    }

    foreach (ShamelaUpdaterTask book, shaBooks) {
        addTask(book);
    }

    foreach (ShamelaUpdaterTask book, libBooks) {
        addTask(book);
    }
}

void ShamelaUpdater::addTask(ShamelaUpdaterTask task)
{
    foreach(ShamelaUpdaterTask t, m_tasks) {
        if((t.bookID == task.bookID || t.bookName == task.bookName)
                && t.bookVersion != task.bookVersion) {
            m_tasks.removeAll(t);
            task.task = ShamelaUpdaterTask::Update;
            m_tasks.append(task);

            return;
        }
    }

    m_tasks.append(task);
}

QList<QStandardItem *> ShamelaUpdater::getTaskItems()
{
    QList<QStandardItem *> items;

    foreach(ShamelaUpdaterTask task, m_tasks) {
        QStandardItem *item = new QStandardItem();
        item->setText(task.bookName);
        item->setData(task.toString(), taskStringRole);

        switch(task.task) {
        case ShamelaUpdaterTask::Add:
            item->setToolTip(QObject::tr("كتاب جديد"));
            item->setIcon(QIcon(":/data/images/add.png"));
            break;
        case ShamelaUpdaterTask::Delete:
            item->setToolTip(QObject::tr("حذف الكتاب"));
            item->setIcon(QIcon(":/data/images/remove.png"));
            break;
        case ShamelaUpdaterTask::Update:
            item->setToolTip(QObject::tr("تحديث الكتاب"));
            item->setIcon(QIcon(":/data/images/refresh.png"));
            break;
        }

        items.append(item);
    }

    return items;
}

QList<int> ShamelaUpdater::getBooksToAdd()
{
    QSet<int> list;

    foreach(ShamelaUpdaterTask task, m_tasks) {
        if(task.task == ShamelaUpdaterTask::Add || task.task == ShamelaUpdaterTask::Update)
            list.insert(task.bookID);
    }

    return list.toList();
}

QList<int> ShamelaUpdater::getBooksToDelete()
{
    QSet<int> list;

    foreach(ShamelaUpdaterTask task, m_tasks) {
        if(task.task == ShamelaUpdaterTask::Delete || task.task == ShamelaUpdaterTask::Update)
            list.insert(task.bookID);
    }
    return list.toList();
}

bool ShamelaUpdater::removeTask(ShamelaUpdaterTask task)
{
    if(!m_tasks.removeAll(task)) {
        qDebug("Can't find this task!");
        return false;
    }

    return true;
}
