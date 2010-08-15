#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <QThread>
#include "common.h"
#include "arabicanalyzer.h"

class IndexingThread : public QThread
{
    Q_OBJECT
public:
    IndexingThread();
    void run();
    void stop() { m_stopIndexing = true; }
    void setOptions(bool optimizeIndex, int ramSize, int maxDoc);

protected:
    void indexBook(IndexWriter *writer, const QString &bookID, const QString &bookPath);
    void startIndexing();

signals:
    void fileIndexed(const QString &bookName);
    void indexingError();

protected:
    bool m_stopIndexing;
    bool m_optimizeIndex;
    int m_ramSize;
    int m_maxDoc;
};

#endif // INDEXTHREAD_H
