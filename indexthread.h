#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include <QThread>
#include "arabicanalyzer.h"
#include "common.h"

class IndexBookThread : public QThread
{
    Q_OBJECT
public:
    IndexBookThread();
    ~IndexBookThread();
    void indexCat();
    void FileDocument(const QString &id, const QString &bookid, const QString &text, Document *doc);
    void indexBoook(const QString &bookID, const QString &bookName, const QString &bookPath,
                    IndexWriter *writer);
    void run();
    void stop();
    QString randFolderName(int len, const QString &prefix=0);
    void setCat(int cat) { m_currentCat = cat; }
    void setOptions(int ramSize, bool optimIndex) { m_ramFlushSize = ramSize; m_optimizeIndex =optimIndex; }

signals:
    void bookIsIndexed(const QString &bookName);
    void doneCatIndexing(const QString &indexFolder);

protected:
    QSqlDatabase indexDB;
    QSqlQuery *inexQuery;
    int m_currentCat;
    bool m_indexing;
    bool m_stop;
    int m_ramFlushSize;
    bool m_optimizeIndex;
    QMutex m_mutex;
    QWaitCondition m_gotBookToIndex;
    QString m_bookID;
    QString m_bookName;
    QString m_indexPath;
};

#endif // INDEXTHREAD_H
