#include "indexoptimizer.h"
#include "cl_common.h"
#include <QTime>

IndexOptimizer::IndexOptimizer(QObject *parent) :
    QThread(parent)
{
    m_indexWriter = 0;
    m_optimizeTime = -1;
    m_optimizeIndex = true;
    m_closeIndex = true;
    m_deleteIndexWriter = false;
}

void IndexOptimizer::run()
{
    if(!m_indexWriter) {
        qWarning("IndexOptimizer index writer not set");
        return;
    }

    QTime time;
    time.start();

    if(m_optimizeIndex) {
        m_indexWriter->optimize(MAX_SEGMENT);
        m_optimizeTime = time.elapsed();

        qDebug("IndexOptimizer index optimized");
    }

    if(m_closeIndex) {
        m_indexWriter->close();
        qDebug("IndexOptimizer index writer closed");
    }

    if(m_deleteIndexWriter) {
        _CLDELETE(m_indexWriter);
        qDebug("IndexOptimizer index writer deleted");
    }
}

lucene::index::IndexWriter *IndexOptimizer::indexWriter() const
{
    return m_indexWriter;
}

void IndexOptimizer::setIndexWriter(lucene::index::IndexWriter *indexWriter)
{
    m_indexWriter = indexWriter;
}
int IndexOptimizer::optimizeTime() const
{
    return m_optimizeTime;
}
bool IndexOptimizer::closeIndex() const
{
    return m_closeIndex;
}

bool IndexOptimizer::optimizeIndex() const
{
    return m_optimizeIndex;
}

bool IndexOptimizer::deleteIndexWriter() const
{
    return m_deleteIndexWriter;
}

void IndexOptimizer::setCloseIndex(bool closeIndex)
{
    m_closeIndex = closeIndex;
}

void IndexOptimizer::setOptimizeIndex(bool optimizeIndex)
{
    m_optimizeIndex = optimizeIndex;
}

void IndexOptimizer::setDeleteIndexWriter(bool deleteIndexWriter)
{
    m_deleteIndexWriter = deleteIndexWriter;
}

