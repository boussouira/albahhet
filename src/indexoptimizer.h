#ifndef INDEXOPTIMIZER_H
#define INDEXOPTIMIZER_H

#include <QThread>

namespace lucene {
    namespace index {
        class IndexWriter;
    }
}

class IndexOptimizer : public QThread
{
    Q_OBJECT
public:
    IndexOptimizer(QObject *parent = 0);

    void run();

    lucene::index::IndexWriter *indexWriter() const;
    void setIndexWriter(lucene::index::IndexWriter *indexWriter);

    int optimizeTime() const;

    bool closeIndex() const;
    bool optimizeIndex() const;
    bool deleteIndexWriter() const;

    void setCloseIndex(bool closeIndex);
    void setOptimizeIndex(bool optimizeIndex);
    void setDeleteIndexWriter(bool deleteIndexWriter);

protected:
    lucene::index::IndexWriter *m_indexWriter;
    int m_optimizeTime;
    bool m_optimizeIndex;
    bool m_closeIndex;
    bool m_deleteIndexWriter;
};

#endif // INDEXOPTIMIZER_H
