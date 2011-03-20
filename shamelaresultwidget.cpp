#include "shamelaresultwidget.h"
#include "ui_shamelaresultwidget.h"

#include "common.h"
#include "shamelaresult.h"
#include "shamelasearcher.h"
#include "webview.h"

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
    m_webView = new WebView(this);
    m_webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->mainVerticalLayout->insertWidget(0, m_webView);

    ui->progressBar->hide();

    m_colors.append("#ffff63");
    m_colors.append("#a5ffff");
    m_colors.append("#ff9a9c");
    m_colors.append("#9cff9c");
    m_colors.append("#ef86fb");

    m_currentShownId = 0;

    m_webView->installEventFilter(this);

    connect(m_webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(populateJavaScriptWindowObject()));
}

ShamelaResultWidget::~ShamelaResultWidget()
{
    m_booksName.clear();
    delete ui;

    if(m_searcher->isRunning())
        m_searcher->wait();

    delete m_searcher;
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
    QString appPath(QString("file:///%1").arg(qApp->applicationDirPath()));

    m_webView->setHtml(QString("<html><head><title></title>"
                                 "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>"
                                 "<link href=\"%1/data/default.css\" rel=\"stylesheet\" type=\"text/css\"/>"
                                 "</head>"
                                 "<body></body>"
                                 "<script type=\"text/javascript\" src=\"%1/data/jquery-1.4.2.min.js\" />"
                                 "<script type=\"text/javascript\" src=\"%1/data/shamela.js\" />"
                                 "</html>")
                         .arg(appPath));

    showNavigationButton(false);

    ui->progressBar->setMaximum(0);
    ui->progressBar->show();

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

    ui->progressBar->hide();
}

void ShamelaResultWidget::fetechStarted()
{
    m_webView->execJS("fetechStarted();");
    showNavigationButton(false);
    ui->progressBar->setMaximum(m_searcher->resultsPeerPage());
    ui->progressBar->setValue(0);
    ui->progressBar->show();
}

void ShamelaResultWidget::fetechFinnished()
{
    ShamelaSearcher *search = qobject_cast<ShamelaSearcher *>(sender());
    if(search) {
        setPageCount(search->currentPage(), search->resultsCount());
    }

    m_webView->execJS("handleEvents();");
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->progressBar->hide();
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
        str = trUtf8("احتمالات البحث كثيرة جدا");
    else if(id == CL_ERR_CorruptIndex || id == CL_ERR_IO){
        str = trUtf8("الفهرس غير سليم");
        desc = what;
    }

    m_webView->execJS(QString("searchException('%1', '%2');")
                                                         .arg(str)
                                                         .arg(desc));
    ui->progressBar->hide();
}

void ShamelaResultWidget::populateJavaScriptWindowObject()
{
    m_webView->page()->mainFrame()->addToJavaScriptWindowObject("resultWidget", this);
}

QString ShamelaResultWidget::cleanString(QString str)
{
    str.replace(QRegExp("[\\x0627\\x0622\\x0623\\x0625]"), "[\\x0627\\x0622\\x0623\\x0625]");//ALEFs
    str.replace(QRegExp("[\\x0647\\x0629]"), "[\\x0647\\x0629]"); //TAH_MARBUTA -> HEH

    return str;
}

QString ShamelaResultWidget::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(m_indexInfo->shamelaPath()).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(m_indexInfo->shamelaPath()).arg(archive);
}

QString ShamelaResultWidget::hiText(const QString &text, const QString &strToHi)
{
    QStringList regExpStr = buildRegExp(strToHi);
    QString finlStr  = text;
    int color = 0;
    bool useColors = (regExpStr.size() <= m_colors.size());

    foreach(QString regExp, regExpStr)
        finlStr.replace(QRegExp(cleanString(regExp)),
                        QString("<b style=\"background-color:%1\">\\1</b>")
                        .arg(m_colors.at(useColors ? color++ : color)));

//    if(!useColors)
//        finlStr.replace(QRegExp("<\\/b>([\\s])<b style=\"background-color:[^\"]+\">"), "\\1");

    return finlStr;
}

QStringList ShamelaResultWidget::buildRegExp(const QString &str)
{
    QString text = str;
    text.remove(QRegExp(trUtf8("[\\x064B-\\x0652\\x0600\\x061B-\\x0620،]")));

    QStringList strWords = text.split(QRegExp(trUtf8("[\\s;,.()\"'{}\\[\\]]")), QString::SkipEmptyParts);
    QStringList regExpList;
    QChar opPar('(');
    QChar clPar(')');
    foreach(QString word, strWords)
    {
        QString regExpStr;
        regExpStr.append("\\b");
        regExpStr.append(opPar);

        for (int i=0; i< word.size();i++) {
            if(word.at(i) == QChar('~'))
                regExpStr.append("[\\S]*");
            else if(word.at(i) == QChar('*'))
                regExpStr.append("[\\S]*");
            else if(word.at(i) == QChar('?'))
                regExpStr.append("\\S");
            else if( word.at(i) == QChar('"') || word.at(i) == opPar || word.at(i) == opPar )
                continue;
            else {
                regExpStr.append(word.at(i));
                regExpStr.append(trUtf8("[ًٌٍَُِّْ]*"));
            }
        }

        regExpStr.append(clPar);
        regExpStr.append("\\b");
        regExpList.append(regExpStr);
    }

    return regExpList;
}

QString ShamelaResultWidget::abbreviate(QString str, int size) {
        if (str.length() <= size-3)
                return str;
        str.simplified();
        int index = str.lastIndexOf(' ', size-3);
        if (index <= -1)
                return "";
        return str.left(index).append("...");
}

QString ShamelaResultWidget::getTitleId(const QSqlDatabase &db, int pageID, int archive, int bookID)
{
    QSqlQuery m_bookQuery(db);
    bool exec;

    exec = m_bookQuery.exec(QString("SELECT TOP 1 tit FROM %1 WHERE id <= %2 ORDER BY id DESC")
                     .arg((!archive) ? "title" : QString("t%1").arg(bookID))
                     .arg(pageID));

    if(!exec)
        SQL_ERROR(m_bookQuery.lastError().text());

    return m_bookQuery.first() ? m_bookQuery.value(0).toString() : QString();
}

QString ShamelaResultWidget::getBookName(int bookID)
{
    QString name = m_booksName.value(bookID, "");
    if(!name.isEmpty()) {
        return name;
    } else {
        QSqlQuery m_bookQuery(QSqlDatabase::database("shamelaBook"));
        bool exec;

        exec = m_bookQuery.exec(QString("SELECT bk FROM 0bok WHERE bkid = %1").arg(bookID));

        if(m_bookQuery.first()){
            QString bookName = m_bookQuery.value(0).toString();
            m_booksName.insert(bookID, bookName);
            return bookName;
        }
        else {
            if(!exec)
                SQL_ERROR(m_bookQuery.lastError().text());
            return QString();
        }
    }
}

void ShamelaResultWidget::setPageCount(int current, int count)
{
    int start = (current * m_searcher->resultsPeerPage()) + 1 ;
    int end = qMax(1, (current * m_searcher->resultsPeerPage()) + m_searcher->resultsPeerPage());
    end = (count >= end) ? end : count;
    ui->labelNav->setText(trUtf8("%1 - %2 من %3 نتيجة")
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

QString ShamelaResultWidget::getPage(QString href)
{
    QUrl url(href);
    int rid = url.queryItems().at(0).second.toInt();
    int bookID = url.queryItems().at(1).second.toInt();
    int archive = url.queryItems().at(2).second.toInt();

    m_currentBookId = bookID;

    QString text;
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "disBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(buildFilePath(QString::number(bookID), archive));
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            DB_OPEN_ERROR(buildFilePath(QString::number(bookID), archive));

            return QString("Error: Cannot open database.");
        }
        QSqlQuery m_bookQuery(m_bookDB);

//        m_bookQuery.exec(QString("SELECT TOP 1 id, nass, page, part FROM %1 "
//                                 "WHERE id <= %2 ORDER BY id DESC")
        m_bookQuery.exec(QString("SELECT id, nass, page, part FROM %1 WHERE id = %2")
                         .arg((!archive) ? "book" : QString("b%1").arg(bookID))
                         .arg(rid));
        if(m_bookQuery.first()) {
            text = m_bookQuery.value(1).toString();

            m_currentShownId = m_bookQuery.value(0).toInt();
            m_currentPage = m_bookQuery.value(2).toInt();
            m_currentPart = m_bookQuery.value(3).toInt();
        } else {
            qWarning("No page at: %d", rid);
        }
        text.replace(QRegExp("[\\r\\n]"),"<br/>");
    }
    QSqlDatabase::removeDatabase("disBook");

    clearShorts(text);
    text = hiText(text, m_searcher->queryString());

    return text;
}

QString ShamelaResultWidget::currentBookName()
{
    return getBookName(m_currentBookId);
}
QString ShamelaResultWidget::baseUrl()
{
    return QString("file:///%1").arg(qApp->applicationDirPath());
}

QString ShamelaResultWidget::formNextUrl(QString href)
{
    QUrl url(href);
    int rid = m_currentShownId+1;
    int bookID = url.queryItems().at(1).second.toInt();
    int archive = url.queryItems().at(2).second.toInt();

    return QString("http://localhost/book.html?id=%1&bookid=%2&archive=%3")
            .arg(rid)
            .arg(bookID)
            .arg(archive);
}

QString ShamelaResultWidget::formPrevUrl(QString href)
{
    QUrl url(href);
    int rid = m_currentShownId-1;
    int bookID = url.queryItems().at(1).second.toInt();
    int archive = url.queryItems().at(2).second.toInt();

    return QString("http://localhost/book.html?id=%1&bookid=%2&archive=%3")
            .arg(rid)
            .arg(bookID)
            .arg(archive);
}

void ShamelaResultWidget::updateNavgitionLinks(QString href)
{
    m_webView->execJS(QString("updateLinks('%1', '%2');")
                                                         .arg(formNextUrl(href))
                                                         .arg(formPrevUrl(href)));

    m_webView->execJS(QString("updateInfoBar('%1', '%2', '%3');")
                                                         .arg(currentBookName())
                                                         .arg(currentPage())
                                                         .arg(currentPart()));
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
