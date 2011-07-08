#ifndef QURANINDEXER_H
#define QURANINDEXER_H

#include <qthread.h>
#include "arabicanalyzer.h"

class BooksDB;
class BookInfo;
class IndexInfo;

class QuranIndexer : public QThread
{
    Q_OBJECT
public:
    QuranIndexer();
    void setBookDB(BooksDB *bookDB) { m_bookDB = bookDB; }
    void setWirter(IndexWriter* writer) { m_writer = writer;}
    void setIndexInfo(IndexInfo* info) { m_indexInfo = info;}
    void run();

protected:
    void startIndexing();
    void indexQuran();

signals:
    void currentSoraName(const QString &bookName);
    void currentBookMax(int max);
    void currentBookProgress(int value);
    void indexingError();

protected:
    BooksDB *m_bookDB;
    IndexWriter* m_writer;
    IndexInfo* m_indexInfo;
    int m_prevSora;
    QList<QString> m_sowarList;
};

#endif // QURANINDEXER_H
