#ifndef BOOKSDB_H
#define BOOKSDB_H

#include "indexinfo.h"
#include <qthread.h>
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

class BooksDB : public QThread
{
public:
    BooksDB();
    ~BooksDB();
    BookInfo *next();
    void setIndexInfo(IndexInfo* info) { m_indexInfo = info;}
    void openIndexDB();
    void openShamelaDB();
    void queryBooksToIndex();
    void importBooksListFromShamela();
    void run();
    void clear();

protected:
    IndexInfo* m_indexInfo;
    QSqlDatabase m_indexDB;
    QSqlQuery *m_indexQuery;
    QSqlDatabase m_shamelaDB;
    QSqlQuery *m_shamelaQuery;
    bool m_indexDbIsOpen;
    bool m_shamelaDbIsOpen;
    QMutex m_mutex;
};

#endif // BOOKSDB_H
