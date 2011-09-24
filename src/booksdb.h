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
class QStandardItem;
class QStandardItemModel;

class BooksDB : public QThread
{
public:
    BooksDB();
    ~BooksDB();
    BookInfo *getBookInfo(int id);
    BookInfo *next();
    void setIndexInfo(IndexInfo* info);
    IndexInfo *indexInfo() { return m_indexInfo; }
    void openIndexDB();
    void openShamelaDB();
    void openShamelaSpecialDB();
    void queryBooksToIndex();
    void queryBooksToIndex(QList<int> ids);
    void importBooksListFromShamela();
    void run();
    void clear();
    void close();
    void setBookIndexed(QSet<int> books);
    void getAuthorFromShamela(QSet<int> author);
    void loadSowarNames();
    QString getSoraName(int soraNumber);

    QList<int> getShamelaIds();
    QList<int> getSavedIds();
    QList<int> getAuthorBooks(int auth);

    QStringList addBooks(QList<int> shaIds);
    QStringList removeBooks(QList<int> savedIds);

    QStandardItemModel *getBooksListModel();
    QStandardItemModel *getCatsListModel();
    QStandardItemModel *getAuthorsListModel();
    void booksCat(QStandardItem *parentNode, int catID);

    QList<QPair<QString, QString> > getBookShoorts(int bid);

    QStringList connections();

    static const int idRole = Qt::UserRole + 2;
    static const int typeRole = Qt::UserRole + 3;
    static const int Categorie = 1;
    static const int Book = 2;

protected:
    IndexInfo* m_indexInfo;
    QSqlDatabase m_indexDB;
    QSqlDatabase m_shamelaDB;
    QSqlDatabase m_shamelaSpecialDB;
    QSqlQuery *m_indexQuery;
    QSqlQuery *m_shamelaQuery;
    QSqlQuery *m_shamelaSpecialQuery;
    QMutex m_mutex;
    QHash<int, BookInfo*> m_bookInfoHash;
    QHash<int, QList<QPair<QString, QString> > > m_savedShoorts;
    QStringList m_sowarNames;
};

#endif // BOOKSDB_H
