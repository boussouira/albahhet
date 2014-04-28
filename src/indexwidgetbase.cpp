#include "indexwidgetbase.h"
#include "common.h"
#include "cl_common.h"
#include "bookinfo.h"
#include "indexesmanager.h"
#include "arabicanalyzer.h"

#include <qmessagebox.h>
#include <qpushbutton.h>
#include <Windows.h>

IndexWidgetBase::IndexWidgetBase(QWidget *parent) : QWidget(parent)
{
}

void IndexWidgetBase::openIndexWriter(QString indexPath)
{
    QSettings settings;
    int ramSize = settings.value("ramSize", 100).toInt();

    ArabicAnalyzer *analyzer = new ArabicAnalyzer();

    m_writer = _CLNEW IndexWriter( qPrintable(indexPath) ,analyzer, true);

    m_writer->setUseCompoundFile(false);
    m_writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
    m_writer->setRAMBufferSizeMB(ramSize);
    m_writer->setMergeFactor(25);
}

void IndexWidgetBase::closeIndexWriter(bool close)
{
    if(close)
        m_writer->close();

    _CLDELETE(m_writer);
}

void IndexWidgetBase::enableHibernateMode(bool enable)
{
    if(enable) {
        SetThreadExecutionState(ES_CONTINUOUS);
    } else {
        if(SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_CONTINUOUS) == NULL) {
            qWarning("Error when preventing system from going to hibernate mode");
        }
    }
}

IndexesManager *IndexWidgetBase::indexesManager() const
{
    return m_indexesManager;
}

void IndexWidgetBase::setIndexesManager(IndexesManager *indexesManager)
{
    m_indexesManager = indexesManager;
}

lucene::index::IndexWriter *IndexWidgetBase::indexWriter() const
{
    return m_writer;
}

