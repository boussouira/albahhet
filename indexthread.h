#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <QThread>
#include "mainwindow.h"

class IndexThread : public QThread
{
    Q_OBJECT
public:
    IndexThread();
    void run();

protected:
    void indexBook(IndexWriter *writer, const QString &bookID, const QString &bookPath);
    Document* FileDocument(const QString &id, const QString &bookid, const QString &text);
    void startIndexing();

signals:
    void fileIndexed(const QString &bookName);
};

#endif // INDEXTHREAD_H
