#ifndef SHAMELAINDEXER_H
#define SHAMELAINDEXER_H

#include <qthread.h>
#include <qset.h>
#include "arabicanalyzer.h"

class BooksDB;
class BookInfo;
class IndexInfo;

class ShamelaIndexer : public QThread
{
    Q_OBJECT
public:
    ShamelaIndexer();
    void setBookDB(BooksDB *bookDB) { m_bookDB = bookDB; }
    void setWirter(IndexWriter* writer) { m_writer = writer;}
    void setIndexInfo(IndexInfo* info) { m_indexInfo = info;}
    void run();

public slots:
    void skipCurrentBook();

protected:
    void indexBook(BookInfo *book);
    void startIndexing();

signals:
    void currentBookName(const QString &bookName);
    void currentBookMax(int max);
    void currentBookProgress(int value);
    void indexingError();

protected:
    BooksDB *m_bookDB;
    IndexWriter* m_writer;
    IndexInfo* m_indexInfo;
    bool m_skipCurrent;
    int m_threadId;
    QSet<int> m_indexedBooks;
    QSet<int> m_authors;
};

#endif // SHAMELAINDEXER_H
