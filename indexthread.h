#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <QThread>
#include "common.h"
#include "indexthread.h"
#include "arabicanalyzer.h"
#include "booksdb.h"
#include "indexinfo.h"

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
    void indexBook(const QString &bookID, const QString &bookPath, const QString &archive);
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
