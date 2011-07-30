#include "quransearchwidget.h"
#include "ui_quransearchwidget.h"
#include "webview.h"
#include <qtextstream.h>
#include <qsettings.h>
#include <qmessagebox.h>

QuranSearchWidget::QuranSearchWidget(QWidget *parent) :
    AbstractSearchWidget(parent),
    ui(new Ui::QuranSearchWidget)
{
    ui->setupUi(this);

    m_searcher = new QuranSearcher(this);
    m_webView = new WebView(IndexInfo::QuranIndex, this);

    ui->verticalLayout->insertWidget(1, m_webView);
    ui->lineEdit->setText(tr("الله"));

    ui->progressWidget->hide();

    loadSettings();

    connect(m_searcher, SIGNAL(gotResult(QuranResult*)), SLOT(gotResult(QuranResult*)));
    connect(m_searcher, SIGNAL(startSearching()), SLOT(searchStarted()));
    connect(m_searcher, SIGNAL(doneSearching()), SLOT(searchFinnished()));
    connect(m_searcher, SIGNAL(startFeteching()), SLOT(fetechStarted()));
    connect(m_searcher, SIGNAL(doneFeteching()), SLOT(fetechFinnished()));
    connect(m_searcher, SIGNAL(gotException(QString, int)), SLOT(gotException(QString, int)));
}

QuranSearchWidget::~QuranSearchWidget()
{
    delete m_searcher;
    delete m_webView;
    delete ui;
}

void QuranSearchWidget::setIndexInfo(IndexInfo *info)
{
    m_indexInfo = info;
    m_searcher->setIndexInfo(m_indexInfo);
}

void QuranSearchWidget::setBooksDb(BooksDB *db)
{
    m_booksDB = db;
    m_searcher->setBooksDb(m_booksDB);
}

void QuranSearchWidget::setTabWidget(TabWidget *tabWidget)
{
    m_tabWidget = tabWidget;
}

void QuranSearchWidget::indexChanged()
{
}

void QuranSearchWidget::saveSettings()
{
}

void QuranSearchWidget::on_pushButton_clicked()
{
    if(ui->lineEdit->text().simplified().isEmpty()){
        QMessageBox::warning(this,
                             tr("البحث"),
                             tr("يجب ان تدخل نصا للبحث"));
        return;
    }

    m_searcher->setSearchText(ui->lineEdit->text());
    m_searcher->setResultsPeerPage(m_resultParPage);
    m_searcher->start();
}

void QuranSearchWidget::on_lineEdit_returnPressed()
{
    on_pushButton_clicked();
}

void QuranSearchWidget::gotResult(QuranResult *result)
{
    m_webView->execJS(QString("addResult('%1')").arg(result->toHtml()));
    ui->progressBar->setValue(ui->progressBar->value()+1);
}

void QuranSearchWidget::searchStarted()
{
    m_webView->init();
    m_webView->execJS("searchStarted();");

    showNavigationButton(false);
    ui->progressBar->setMaximum(0);
    ui->progressWidget->show();
}

void QuranSearchWidget::searchFinnished()
{
    m_webView->execJS("searchFinnished();");

    if(m_searcher->resultCount() > 0) {
        m_webView->execJS(QString("setSearchTime(%1);").arg(m_searcher->searchTime()));
    } else {
        m_webView->execJS("noResultFound();");
        showNavigationButton(false);
    }

    ui->progressWidget->hide();
}

void QuranSearchWidget::fetechStarted()
{
    m_webView->execJS("fetechStarted();");

    showNavigationButton(false);
    ui->progressBar->setMaximum(m_searcher->resultsPeerPage());
    ui->progressBar->setValue(0);
    ui->progressWidget->show();
}

void QuranSearchWidget::fetechFinnished()
{
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->progressWidget->hide();
    updateNavigation(m_searcher->currentPage(), m_searcher->resultCount());
    showNavigationButton(true);
}

void QuranSearchWidget::gotException(QString what, int id)
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
}

void QuranSearchWidget::populateJavaScriptWindowObject()
{
}

void QuranSearchWidget::showNavigationButton(bool show)
{
    ui->widgetNavigationButtons->setVisible(show);
}

void QuranSearchWidget::loadSettings()
{
    QSettings settings;
    m_resultParPage = settings.value("resultPeerPage", 10).toInt();

    if(m_resultParPage <= 0)
        m_resultParPage = 10;
}

void QuranSearchWidget::on_buttonGoNext_clicked()
{
    m_searcher->nextPage();
}

void QuranSearchWidget::on_buttonGoPrev_clicked()
{
    m_searcher->prevPage();
}

void QuranSearchWidget::on_buttonGoLast_clicked()
{
    m_searcher->lastPage();
}

void QuranSearchWidget::on_buttonGoFirst_clicked()
{
    m_searcher->firstPage();
}

void QuranSearchWidget::updateNavigation(int current, int count)
{
    int start = (current * m_searcher->resultsPeerPage()) + 1 ;
    int end = qMax(1, (current * m_searcher->resultsPeerPage()) + m_searcher->resultsPeerPage());
    end = (count >= end) ? end : count;
    ui->labelNav->setText(tr("%1 - %2 من %3 نتيجة")
                       .arg(start)
                       .arg(end)
                       .arg(count));
    buttonStat(current, m_searcher->pagesCount());
}

void QuranSearchWidget::buttonStat(int currentPage, int pageCount)
{
    bool back = (currentPage > 0);
    bool next = (currentPage < pageCount-1);

    ui->buttonGoPrev->setEnabled(back);
    ui->buttonGoFirst->setEnabled(back);

    ui->buttonGoNext->setEnabled(next);
    ui->buttonGoLast->setEnabled(next);
}
