#include "searchfield.h"
#include "common.h"

#include <QVariant>

SearchField::SearchField()
{
    m_query = 0;
}

SearchField::~SearchField()
{
    close();
}

void SearchField::openDB()
{
    if(!m_indexDB.isOpen()) {
        connName = QString("SearchField_%1").arg(m_indexInfo->id());
        QString dbPath = m_indexInfo->indexDbPath();

        m_indexDB = QSqlDatabase::addDatabase("QSQLITE", connName);
        m_indexDB.setDatabaseName(dbPath);

        if (!m_indexDB.open()) {
            DB_OPEN_ERROR(dbPath);
        }

        m_query = new QSqlQuery(m_indexDB);
    }
}

QList<int> SearchField::getFieldBooks(int fid)
{
    QList<int> list;

    m_query->prepare("SELECT bookID FROM fieldsBooks "
                     "WHERE id = ?");
    m_query->bindValue(0, fid);
    m_query->exec();

    while(m_query->next()) {
        list.append(m_query->value(0).toInt());
    }

    return list;
}

void SearchField::removeield(int fid)
{
    m_query->prepare("DELETE FROM fields WHERE id = ?");
    m_query->bindValue(0, fid);
    if(m_query->exec()) {
        m_query->prepare("DELETE FROM fieldsBooks WHERE id = ?");
        m_query->bindValue(0, fid);
        m_query->exec();
    }
}

void SearchField::setFieldName(QString name, int fid)
{
    if(name.isEmpty())
        return;

    m_query->prepare("UPDATE fields SET name = ? "
                     "WHERE id = ?");
    m_query->bindValue(0, name);
    m_query->bindValue(1, fid);
    m_query->exec();
}

bool SearchField::addField(int indexID, QString name, QList<int> list)
{
    if(name.isEmpty() || list.isEmpty())
        return false;

    m_indexDB.transaction();

    m_query->prepare("INSERT INTO fields (id, indexID, name) "
                     "VALUES(NULL, ?, ?)");
    m_query->bindValue(0, indexID);
    m_query->bindValue(1, name);
    m_query->exec();

    int fid = m_query->lastInsertId().toInt();
    foreach(int bid, list) {
        m_query->prepare("INSERT INTO fieldsBooks (id, bookID) "
                         "VALUES(?, ?)");
        m_query->bindValue(0, fid);
        m_query->bindValue(1, bid);
        m_query->exec();
    }

    return m_indexDB.commit();
}

QList<SearchFieldInfo*> SearchField::getFieldsNames(int fid)
{
    QList<SearchFieldInfo*> list;
    m_query->prepare("SELECT id, name FROM fields "
                     "WHERE indexID = ?");
    m_query->bindValue(0, fid);
    m_query->exec();

    while(m_query->next()) {
        SearchFieldInfo *field = new SearchFieldInfo();
        field->fieldID = m_query->value(0).toInt();
        field->name = m_query->value(1).toString();

        list.append(field);
    }

    return list;
}

QList<SearchFieldInfo*> SearchField::getFieldsNames()
{
    return getFieldsNames(m_indexInfo->id());
}

void SearchField::setIndexInfo(IndexInfo *index)
{
    m_indexInfo = index;

    close();
    openDB();
}

void SearchField::close()
{
    if(m_indexDB.isOpen()) {
        if(m_query) {
            delete m_query;
            m_query = 0;
        }

        m_indexDB.close();
        m_indexDB = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
    }
}
