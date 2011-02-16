#ifndef BOOKSDB_H
#define BOOKSDB_H

#include <qthread.h>
#include <qmutex.h>
#include <qsqldatabase.h>
#include <qstringlist.h>
#include <qhash.h>
#include <qdebug.h>

class BookInfo;
class IndexInfo;
class QStandardItemModel;
class QStandardItemModel;

class BooksDB : public QThread
{
public:
    BooksDB();
    ~BooksDB();
    BookInfo *getBookInfo(int id);
    BookInfo *next();
    void setIndexInfo(IndexInfo* info);
    void openIndexDB();
    void openShamelaDB();
    void openShamelaSpecialDB();
    void queryBooksToIndex();
    void queryBooksToIndex(QList<int> ids);
    void importBooksListFromShamela();
    void run();
    void clear();
    void close();
    void setBookIndexed(int shaId);
    void getAuthorFromShamela(int id);

    QList<int> getShamelaIds();
    QList<int> getSavedIds();

    int addBooks(QList<int> shaIds);
    int removeBooks(QList<int> savedIds);

    QStandardItemModel *getBooksListModel();
    QStandardItemModel *getCatsListModel();
    QStandardItemModel *getAuthorsListModel();

    QStringList connections();

protected:
    IndexInfo* m_indexInfo;
    QSqlDatabase m_indexDB;
    QSqlQuery *m_indexQuery;
    QSqlDatabase m_shamelaDB;
    QSqlQuery *m_shamelaQuery;
    QSqlDatabase m_shamelaSpecialDB;
    QSqlQuery *m_shamelaSpecialQuery;
    bool m_indexDbIsOpen;
    bool m_shamelaDbIsOpen;
    bool m_shamelaSpecialDbIsOpen;
    QString m_shaConnName;
    QString m_indexConnName;
    QString m_shaSpeConnName;
    QMutex m_mutex;
    QHash<int, BookInfo*> m_bookInfoHash;
    QList<int> m_savedAuthors;
};

#endif // BOOKSDB_H
