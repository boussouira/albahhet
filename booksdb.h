#ifndef BOOKSDB_H
#define BOOKSDB_H

#include <QMutex>
#include <QMutexLocker>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class BookInfo
{
public:
    BookInfo(QString id, QString name, QString path, QString archive):
            m_id(id),
            m_name(name),
            m_path(path),
            m_archive(archive)
    {
    }
    QString id() { return m_id; }
    QString name() { return m_name; }
    QString path() { return m_path; }
    QString arhive() { return m_archive; }
protected:
    QString m_id;
    QString m_name;
    QString m_path;
    QString m_archive;
};

class BooksDB
{
public:
    BooksDB();
    ~BooksDB();
    BookInfo *next();
    void clear();

protected:
    QSqlDatabase m_db;
    QSqlQuery *m_query;
    QMutex m_mutex;
};

#endif // BOOKSDB_H
