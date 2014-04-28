#include "indexwidgetbase.h"
#include "common.h"
#include "cl_common.h"
#include "bookinfo.h"
#include "indexesmanager.h"
#include "arabicanalyzer.h"
#include "mainwindow.h"

#include <qmessagebox.h>
#include <qpushbutton.h>
#include <Windows.h>

// for windows progress bar
#ifndef __GNUC__
#    include <shobjidl.h>
#endif

// Windows 7 SDK required
#ifdef __ITaskbarList3_INTERFACE_DEFINED__

#define WINDOWS_PROGRESS_BAR

namespace {
    int total = 0;
    ITaskbarList3* pITask = 0;
}
#endif

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

void IndexWidgetBase::initInternal()
{
#ifdef WINDOWS_PROGRESS_BAR

    CoInitialize(NULL);
    HRESULT hRes = CoCreateInstance(CLSID_TaskbarList,
                                    NULL,CLSCTX_INPROC_SERVER,
                                    IID_ITaskbarList3,(LPVOID*) &pITask);
     if (FAILED(hRes))
     {
         pITask = 0;
         CoUninitialize();
         return;
     }

     pITask->HrInit();
#endif
}

void IndexWidgetBase::cleanup()
{
#ifdef WINDOWS_PROGRESS_BAR
    if (pITask) {
        pITask->Release();
        pITask = NULL;
        CoUninitialize();
    }
#endif
}

void IndexWidgetBase::setApplicationProgressRange(int min, int max)
{
#ifdef WINDOWS_PROGRESS_BAR
    total = max-min;
#else
    Q_UNUSED(min);
    Q_UNUSED(max);
#endif
}

void IndexWidgetBase::setApplicationProgressValue(int value)
{
#ifdef WINDOWS_PROGRESS_BAR
    if (pITask) {
        const HWND winId = MainWindow::instance()->winId();
        pITask->SetProgressValue ( winId, value, total);
    }
#else
    Q_UNUSED(value);
#endif
}

void IndexWidgetBase::setApplicationProgressVisible(bool visible, bool indeterminated)
{
#ifdef WINDOWS_PROGRESS_BAR
    if (!pITask)
        return;

    const HWND winId = MainWindow::instance()->winId();
    if (visible) {
        if(indeterminated)
            pITask->SetProgressState(winId, TBPF_INDETERMINATE);
        else
            pITask->SetProgressState(winId, TBPF_NORMAL);
    } else {
        pITask->SetProgressState(winId, TBPF_NOPROGRESS);
    }
#else
    Q_UNUSED(visible);
    Q_UNUSED(indeterminated);
#endif
}

lucene::index::IndexWriter *IndexWidgetBase::indexWriter() const
{
    return m_writer;
}

