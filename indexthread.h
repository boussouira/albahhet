#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <QThread>
#include "arabicanalyzer.h"
#include "common.h"

class IndexBookThread;
class IndexingThread : public QThread
{
    Q_OBJECT
public:
    IndexingThread();
    void run();
    void stop() { m_stopIndexing = true; }
    void setOptions(bool optimizeIndex, int ramSize, int maxDoc, int threadsCount);

protected:
    void indexBook(IndexWriter *writer, const QString &bookID, const QString &bookPath);
    void startIndexing();
    void finishIndexing();

public slots:
    void nextBook(IndexBookThread *thread);
    void threadFinishBook(const QString &book);

signals:
    void fileIndexed(const QString &bookName);
    void indexingError();

protected:
    IndexWriter* writer;
    QSqlDatabase indexDB;
    QSqlQuery *inexQuery;
    QSemaphore *m_sem;
    QMutex m_mutex;
    bool m_stopIndexing;
    bool m_optimizeIndex;
    int m_ramSize;
    int m_maxDoc;
    int m_threadCount;
};

class IndexBookThread : public QThread
{
    Q_OBJECT
public:
    IndexBookThread(IndexWriter *writer);
    void setBook(const QString &bookID, const QString &bookName, const QString &bookPath);
    Document* FileDocument(const QString &id, const QString &bookid, const QString &text);
    void indexBoook(const QString &bookID, const QString &bookName, const QString &bookPath);
    void run();
    void stop();

signals:
    void giveNextBook(IndexBookThread *thread);
    void bookIsIndexed(const QString &bookName);

protected:
    IndexWriter *m_writer;
    bool m_indexing;
    bool m_stop;
    QMutex m_mutex;
    QWaitCondition m_gotBookToIndex;
    QString m_bookID;
    QString m_bookName;
    QString m_bookPath;
};

#endif // INDEXTHREAD_H
