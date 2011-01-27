#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <qthread.h>
#include "arabicanalyzer.h"

class BooksDB;
class BookInfo;
class IndexInfo;

class IndexingThread : public QThread
{
    Q_OBJECT
public:
    IndexingThread();
    void setBookDB(BooksDB *bookDB) { m_bookDB = bookDB; }
    void setWirter(IndexWriter* writer) { m_writer = writer;}
    void setIndexInfo(IndexInfo* info) { m_indexInfo = info;}
    void run();
    void stop() { m_stopIndexing = true; }

protected:
    void indexBook(BookInfo *book);
    void startIndexing();

signals:
    void fileIndexed(const QString &bookName);
    void indexingError();

protected:
    BooksDB *m_bookDB;
    IndexWriter* m_writer;
    IndexInfo* m_indexInfo;
    bool m_stopIndexing;
};

#endif // INDEXTHREAD_H
