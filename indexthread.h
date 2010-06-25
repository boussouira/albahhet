#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <QThread>
#include "arabicanalyzer.h"
#include "common.h"

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
