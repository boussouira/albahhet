#include "shamelaresultwidget.h"
#include "ui_shamelaresultwidget.h"

#include "common.h"
#include "shamelaresult.h"
#include "shamelasearcher.h"
#include "webview.h"
#include "shamelabooksreader.h"

#include <qsettings.h>
#include <qfile.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qtextstream.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include <qevent.h>

ShamelaResultWidget::ShamelaResultWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShamelaResultWidget)
{
    ui->setupUi(this);

    m_searcher = new ShamelaSearcher;
    m_bookReader = new ShamelaBooksReader(this);
    m_webView = new WebView(IndexInfo::ShamelaIndex, this);
    m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->mainVerticalLayout->insertWidget(0, m_webView);

    ui->progressWidget->hide();
    connect(m_webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(populateJavaScriptWindowObject()));
}

ShamelaResultWidget::~ShamelaResultWidget()
{

    if(m_searcher->isRunning())
        m_searcher->wait();

    delete ui;
    delete m_searcher;
    DELETE_DB(m_bookReader);
}

void ShamelaResultWidget::setShamelaSearch(ShamelaSearcher *s)
{
    m_searcher = s;
    connect(m_searcher, SIGNAL(gotResult(ShamelaResult*)), SLOT(gotResult(ShamelaResult*)));
    connect(m_searcher, SIGNAL(startSearching()), SLOT(searchStarted()));
    connect(m_searcher, SIGNAL(doneSearching()), SLOT(searchFinnished()));
    connect(m_searcher, SIGNAL(startFeteching()), SLOT(fetechStarted()));
    connect(m_searcher, SIGNAL(doneFeteching()), SLOT(fetechFinnished()));
    connect(m_searcher, SIGNAL(gotException(QString, int)), SLOT(gotException(QString, int)));
    connect(ui->buttonStopFetech, SIGNAL(clicked()), m_searcher, SLOT(stopFeteching()));
}

void ShamelaResultWidget::doSearch()
{
    m_searcher->start();
}

void ShamelaResultWidget::clearResults()
{
    m_searcher->clear();
    m_webView->setHtml("");
    showNavigationButton(false);
}

void ShamelaResultWidget::searchStarted()
{
    m_webView->init();

    showNavigationButton(false);

    ui->progressBar->setMaximum(0);
    ui->progressWidget->show();

    m_webView->execJS("searchStarted();");
}

void ShamelaResultWidget::searchFinnished()
{
    m_webView->execJS("searchFinnished();");

    if(m_searcher->resultsCount() > 0) {
        m_webView->execJS(QString("setSearchTime(%1);")
                          .arg(m_searcher->searchTime()));
    } else {
        m_webView->execJS("noResultFound();");

        showNavigationButton(false);
    }

    ui->progressWidget->hide();
}

void ShamelaResultWidget::fetechStarted()
{
    m_webView->execJS("fetechStarted();");
    showNavigationButton(false);
    ui->progressBar->setMaximum(m_searcher->resultsPeerPage());
    ui->progressBar->setValue(0);
    ui->progressWidget->show();
}

void ShamelaResultWidget::fetechFinnished()
{
    ShamelaSearcher *search = qobject_cast<ShamelaSearcher *>(sender());
    if(search) {
        setPageCount(search->currentPage(), search->resultsCount());
    }

    m_webView->execJS("handleEvents();");
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->progressWidget->hide();
    showNavigationButton(true);
//    writeHtmlResult();
}

void ShamelaResultWidget::gotResult(ShamelaResult *result)
{
    m_webView->execJS(QString("addResult('%1');").arg(result->toHtml()));
    ui->progressBar->setValue(ui->progressBar->value()+1);
}

void ShamelaResultWidget::gotException(QString what, int id)
{
    QString str = what;
    QString desc;

    if(id == CL_ERR_TooManyClauses)
        str = tr("احتمالات البحث كثيرة جدا");
    else if(id == CL_ERR_CorruptIndex || id == CL_ERR_IO){
        str = tr("الفهرس غير سليم");
        desc = what;
    }

    m_webView->execJS(QString("searchException('%1', '%2');")
                                                         .arg(str.replace('\'', "\\'"))
                                                         .arg(desc.replace('\'', "\\'")));
    ui->progressWidget->hide();
}

void ShamelaResultWidget::populateJavaScriptWindowObject()
{
    m_webView->addObject("resultWidget", this);
    m_webView->addObject("bookReader", m_bookReader);
}

void ShamelaResultWidget::setPageCount(int current, int count)
{
    int start = (current * m_searcher->resultsPeerPage()) + 1 ;
    int end = qMax(1, (current * m_searcher->resultsPeerPage()) + m_searcher->resultsPeerPage());
    end = (count >= end) ? end : count;
    ui->labelNav->setText(tr("%1 - %2 من %3 نتيجة")
                       .arg(start)
                       .arg(end)
                       .arg(count));
    buttonStat(current, m_searcher->pageCount());
}

void ShamelaResultWidget::buttonStat(int currentPage, int pageCount)
{
    bool back = (currentPage > 0);
    bool next = (currentPage < pageCount-1);

    ui->buttonGoPrev->setEnabled(back);
    ui->buttonGoFirst->setEnabled(back);

    ui->buttonGoNext->setEnabled(next);
    ui->buttonGoLast->setEnabled(next);
}

void ShamelaResultWidget::openResult(int bookID, int resultID)
{
    m_bookReader->close();

    QSettings settings;
    BookInfo *info = m_booksDb->getBookInfo(bookID);
    ShamelaResult *result = m_searcher->getSavedResult(resultID);

    m_bookReader->setBooksDB(m_booksDb);
    m_bookReader->setBookInfo(info);
    m_bookReader->setResult(result);
    m_bookReader->setStringTohighlight(m_searcher->queryString());
    m_bookReader->setHighLightAll(!settings.value("BooksViewer/highlightOnlyFirst", true).toBool());

    if(!m_bookReader->open())
        qFatal("Can't open book");
}

QString ShamelaResultWidget::baseUrl()
{
    return QString("file:///%1").arg(qApp->applicationDirPath());
}

void ShamelaResultWidget::showNavigationButton(bool show)
{
    ui->widgetNavigationButtons->setVisible(show);
}

void ShamelaResultWidget::on_buttonGoNext_clicked()
{
    m_searcher->nextPage();
}

void ShamelaResultWidget::on_buttonGoPrev_clicked()
{
    m_searcher->prevPage();
}

void ShamelaResultWidget::on_buttonGoLast_clicked()
{
    m_searcher->lastPage();
}

void ShamelaResultWidget::on_buttonGoFirst_clicked()
{
    m_searcher->firstPage();
}

void ShamelaResultWidget::writeHtmlResult()
{
    QFile file("test.html");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << m_webView->html();
}