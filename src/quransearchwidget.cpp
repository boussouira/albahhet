#include "quransearchwidget.h"
#include "ui_quransearchwidget.h"
#include "webview.h"
#include <qtextstream.h>
#include <qsettings.h>

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
    m_searcher->setSearchText(ui->lineEdit->text());
    m_searcher->setResultsPeerPage(m_resultParPage);
    m_searcher->start();
}

void QuranSearchWidget::on_lineEdit_textChanged(const QString &arg1)
{
    /*
    m_searcher->setIndexInfo(m_indexInfo);
    m_searcher->setBooksDb(m_booksDB);
    m_searcher->setSearchText(arg1);
    m_searcher->run();
    m_view->setHtml(tr("%1 -> %2").arg(arg1).arg(m_searcher->resultCount()));

    QFile file("test.html");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << m_view->html();
    */
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
