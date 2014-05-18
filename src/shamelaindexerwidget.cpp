#include "shamelaindexerwidget.h"
#include "shamelaindexerpages.h"
#include "common.h"
#include "shamelaindexinfo.h"
#include "indexesmanager.h"
#include "booksdb.h"

ShamelaIndexerWidget::ShamelaIndexerWidget(QWidget *parent) : IndexWidgetBase(parent)
{
    m_indexInfo = new ShamelaIndexInfo();
    m_bookDB = new BooksDB();

    m_booksCount = 0;
    m_indexedBooksCount = 0;
    m_indexingTime = -1;
    m_optimizeIndexTime = -1;

    m_pages.insert(Page_SelectShamelaPage, new SelectShamelaPage(this));
    m_pages.insert(Page_SelectBooksPage,   new SelectBooksPage(this));
    m_pages.insert(Page_BooksIndexingPage, new BooksIndexingPage(this));
    m_pages.insert(Page_DoneIndexingPage,  new DoneIndexingPage(this));
}

QString ShamelaIndexerWidget::id()
{
    return "shamela.index";
}

QString ShamelaIndexerWidget::title()
{
    return tr("المكتبة الشاملة");
}

QString ShamelaIndexerWidget::description()
{
    return tr("انشاء فهرس للمكتبة الشاملة");
}

QList<QWizardPage *> ShamelaIndexerWidget::pages()
{
    return m_pages;
}

ShamelaIndexInfo *ShamelaIndexerWidget::indexInfo() const
{
    return m_indexInfo;
}

BooksDB *ShamelaIndexerWidget::bookDB() const
{
    return m_bookDB;
}

int ShamelaIndexerWidget::booksCount() const
{
    return m_booksCount;
}

void ShamelaIndexerWidget::setIndexingTime(int indexingTime)
{
    m_indexingTime = indexingTime;
}

void ShamelaIndexerWidget::setOptimizeIndexTime(int optimizeIndexTime)
{
    m_optimizeIndexTime = optimizeIndexTime;
}

void ShamelaIndexerWidget::setIndexedBooksCount(int indexedBooksCount)
{
    m_indexedBooksCount = indexedBooksCount;
}

void ShamelaIndexerWidget::setBooksCount(int booksCount)
{
    m_booksCount = booksCount;
}

void ShamelaIndexerWidget::saveIndexInfo()
{
    m_indexInfo->setType(ShamelaIndexInfo::ShamelaIndex);
    m_indexInfo->generateIndexingInfo();
    m_indexInfo->indexingInfo()->indexingTime = m_indexingTime;
    m_indexInfo->indexingInfo()->optimizingTime = m_optimizeIndexTime;
    m_indexInfo->indexingInfo()->creatTime = QDateTime::currentDateTime().toTime_t();

    m_indexesManager->add(m_indexInfo);

    DELETE_DB(m_bookDB); // We don't need it any more, the mainwindow may open the same databases...
    m_bookDB = new BooksDB();

    emit indexCreated();
}

bool ShamelaIndexerWidget::cancel()
{
    QWizard *wizard = qobject_cast<QWizard*>(parent());
    if(!wizard)
        return true;

//    int currentId = wizard->currentId();
//    if(currentId == -1)
//        return;

    ShamelaPageBase *page = qobject_cast<ShamelaPageBase*>(wizard->currentPage());
    if(!page)
        return true;

    return page->cancel();
}

int ShamelaIndexerWidget::indexingTime() const
{
    return m_indexingTime;
}

int ShamelaIndexerWidget::optimizeIndexTime() const
{
    return m_optimizeIndexTime;
}

int ShamelaIndexerWidget::indexedBooksCount() const
{
    return m_indexedBooksCount;
}
