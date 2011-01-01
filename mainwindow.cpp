#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QWebFrame"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_searcher = new ShamelaSearcher();
    m_currentIndex = new IndexInfo();

    m_fetechProgressBar = new QProgressBar(this);
    m_fetechProgressBar->setTextVisible(false);
    statusBar()->addWidget(m_fetechProgressBar, 0);
    m_fetechProgressBar->hide();

    m_searchTimeLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_searchTimeLabel, 0);
    m_searchTimeLabel->hide();

    m_searchResultsLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_searchResultsLabel, 1);
    m_searchTimeLabel->hide();

    m_resultCount = 0;
    m_resultParPage = 10;
    m_currentShownId = 0;
    m_dbIsOpen = false;

    m_colors.append("#FFFF63");
    m_colors.append("#A5FFFF");
    m_colors.append("#FF9A9C");
    m_colors.append("#9CFF9C");
    m_colors.append("#EF86FB");

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    m_searchQuery = settings.value("lastQuery").toString();
    m_resultParPage = settings.value("resultPeerPage", m_resultParPage).toInt();

    settings.beginGroup("MainWindow");
    resize(settings.value("size", size()).toSize());
    move(settings.value("pos", pos()).toPoint());
    settings.endGroup();
    loadIndexesList();

    ui->lineQuery->setText(m_searchQuery);

    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    ui->lineQuery->setLayoutDirection(Qt::LeftToRight);
    ui->lineQuery->setLayoutDirection(Qt::RightToLeft);

    connect(ui->actionNewIndex, SIGNAL(triggered()), this, SLOT(newIndex()));
    connect(ui->pushSearch, SIGNAL(clicked()), this, SLOT(startSearching()));
    connect(ui->actionIndexInfo, SIGNAL(triggered()), this, SLOT(showStatistic()));
    connect(ui->actionSearchSettings, SIGNAL(triggered()), this, SLOT(displayResultsOptions()));
    connect(ui->webView, SIGNAL(linkClicked(QUrl)), this, SLOT(resultLinkClicked(QUrl)));
    connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(populateJavaScriptWindowObject()));
}

MainWindow::~MainWindow()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.setValue("lastQuery", ui->lineQuery->text());
    settings.setValue("resultPeerPage", m_resultParPage);

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    if(!m_currentIndex->name().isEmpty())
        settings.setValue("current_index",
                          QString("i_%1").arg(IndexInfo::nameHash(m_currentIndex->name())));

    delete ui;
}

void MainWindow::loadIndexesList()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    QStringList list =  settings.value("indexes_list").toStringList();
    bool enableWidgets = true;

    if(!list.isEmpty()) {
        QString current = settings.value("current_index", list.first()).toString();

        for(int i=0; i<list.count(); i++) {
            IndexInfo *info = new IndexInfo();
            settings.beginGroup(list.at(i));
            info->setName(settings.value("name").toString());
            info->setShamelaPath(settings.value("shamela_path").toString());
            info->setPath(settings.value("index_path").toString());
            info->setRamSize(settings.value("ram_size").toInt());
            info->setOptimizeIndex(settings.value("optimizeIndex").toBool());
            settings.endGroup();

            m_indexInfoMap.insert(list.at(i), info);

            QAction *action = new QAction(info->name(), ui->menuIndexesList);
            action->setData(list.at(i));
            connect(action, SIGNAL(triggered()), this, SLOT(changeIndex()));

            ui->menuIndexesList->addAction(action);
        }

        selectIndex(current);

    } else {
        enableWidgets = false;
    }

    ui->lineQuery->setEnabled(enableWidgets);
    ui->pushSearch->setEnabled(enableWidgets);
    ui->menuIndexesList->setEnabled(enableWidgets);
    ui->actionIndexInfo->setEnabled(enableWidgets);
    ui->checkBox->setEnabled(enableWidgets);
    ui->webView->setEnabled(enableWidgets);
}

void MainWindow::selectIndex(QString name)
{
    foreach(QAction *a, ui->menuIndexesList->actions()) {
        if(a->data().toString() == name) {
            selectIndex(a);
            break;
        }
    }

}

void MainWindow::selectIndex(QAction *act)
{
    m_currentIndex = m_indexInfoMap[act->data().toString()];
    act->setCheckable(true);
    act->setChecked(true);

    indexChanged();
}

void MainWindow::changeIndex()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action) {
        if(action->isCheckable()) {
            action->setChecked(true);
            return;
        }

        foreach(QAction *a, ui->menuIndexesList->actions()) {
            a->setChecked(false);
            a->setCheckable(false);
        }

        selectIndex(action);
    }
}

void MainWindow::indexChanged()
{
    m_searcher->clear();
    ui->webView->setHtml("");
    ui->label->setText("");

    ui->pushGoPrev->setEnabled(false);
    ui->pushGoFirst->setEnabled(false);
    ui->pushGoNext->setEnabled(false);
    ui->pushGoLast->setEnabled(false);
}

void MainWindow::updateIndexesMenu()
{
    QList<QAction*> list = ui->menuIndexesList->actions();
    qDeleteAll(list);
    m_indexInfoMap.clear();

    loadIndexesList();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::newIndex()
{
    IndexingDialg *indexDial = new IndexingDialg(this);
    if(indexDial->exec() == QDialog::Accepted)
       updateIndexesMenu();
}

void MainWindow::startSearching()
{
    m_searcher->clear();

    if(!m_dbIsOpen)
        openDB();

    m_searchQuery = ui->lineQuery->text();

    m_searchQuery.replace(QRegExp(trUtf8("ـفق")), "(");
    m_searchQuery.replace(QRegExp(trUtf8("ـغق")), ")");
    m_searchQuery.replace(QRegExp(trUtf8("ـ[أا]و")), "OR");
    m_searchQuery.replace(QRegExp(trUtf8("ـو")), "AND");
    m_searchQuery.replace(QRegExp(trUtf8("ـبدون")), "NOT");
    m_searchQuery.replace(trUtf8("؟"), "?");

    m_searcher = new ShamelaSearcher;
    m_searcher->setIndexInfo(m_currentIndex);
    m_searcher->setQueryString(m_searchQuery);
    m_searcher->setResultsPeerPage(m_resultParPage);
    m_searcher->setsetDefaultOperator(ui->checkBox->isChecked());

    connect(m_searcher, SIGNAL(gotResult(ShamelaResult*)), this, SLOT(gotResult(ShamelaResult*)));
    connect(m_searcher, SIGNAL(startSearching()), this, SLOT(searchStarted()));
    connect(m_searcher, SIGNAL(doneSearching()), this, SLOT(searchFinnished()));
    connect(m_searcher, SIGNAL(startFeteching()), this, SLOT(fetechStarted()));
    connect(m_searcher, SIGNAL(doneFeteching()), this, SLOT(fetechFinnished()));

    /*
    QFile file("out.html");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << ui->webView->page()->mainFrame()->toHtml();
    */
    m_searcher->start();
}

void MainWindow::searchStarted()
{
    QString appPath(QString("file:///%1").arg(qApp->applicationDirPath()));

    ui->webView->setHtml(QString("<html><head><title></title>"
                                 "<link href=\"%1/data/default.css\"  rel=\"stylesheet\" type=\"text/css\" />"
                                 "</head>"
                                 "<body></body>"
                                 "<script type=\"text/javascript\" src=\"%1/data/jquery-1.4.2.min.js\"></script>"
                                 "<script type=\"text/javascript\" src=\"%1/data/scripts.js\"></script>"
                                 "</html>")
                         .arg(appPath));

    ui->label->clear();
    m_searchTimeLabel->clear();
    m_searchResultsLabel->clear();
    m_searchTimeLabel->hide();
    m_searchResultsLabel->hide();

    buttonStat(1, 1);
    ui->pushSearch->setEnabled(false);
    ui->lineQuery->setEnabled(false);
    ui->webView->page()->mainFrame()->evaluateJavaScript("searchStarted()");
}

void MainWindow::searchFinnished()
{
    ui->webView->page()->mainFrame()->evaluateJavaScript("searchFinnished()");
    m_searchTimeLabel->setText(trUtf8("  مدة البحث: %1 ثانية").arg(m_searcher->searchTime()/1000.0));
    m_searchResultsLabel->setText(trUtf8("  نتائج البحث: %1").arg(m_searcher->resultsCount()));
    m_searchTimeLabel->show();
    m_searchResultsLabel->show();
}

void MainWindow::fetechStarted()
{
    ui->pushSearch->setEnabled(false);
    ui->lineQuery->setEnabled(false);

    ui->webView->page()->mainFrame()->evaluateJavaScript("fetechStarted()");
    m_fetechProgressBar->setMaximum(m_searcher->resultsPeerPage());
    m_fetechProgressBar->setValue(0);
    m_fetechProgressBar->show();
}

void MainWindow::fetechFinnished()
{
    ShamelaSearcher *search = qobject_cast<ShamelaSearcher *>(sender());
    if(search) {
        setPageCount(search->currentPage()+1, search->pageCount());
    }

    ui->webView->page()->mainFrame()->evaluateJavaScript("handleEvents()");
    m_fetechProgressBar->setValue(m_fetechProgressBar->maximum());
    m_fetechProgressBar->hide();

    ui->pushSearch->setEnabled(true);
    ui->lineQuery->setEnabled(true);
}

void MainWindow::gotResult(ShamelaResult *result)
{
    QString resultString = trUtf8("<div class=\"result %1\">"
                               "<h3>%2</h3>"
                               "<span class=\"progSpan\" style=\"width: %10px;\">"
                               "<span class=\"progSpanContainre\"></span>"
                               "</span>"
                               "<a class=\"bookLink\" href=\"http://localhost/book.html?id=%3&bookid=%8&archive=%9\">%4</a>"
                               "<p style=\"margin: 5px 0px 0px;\"> كتاب: <span class=\"bookName\">%5</span>"
                               "<span style=\"float: left;\">الصفحة: <span style=\"margin-left: 7px;\">%6</span>  الجزء: <span>%7</span></span>"
                               "</p></div>")
                        .arg(result->bgColor())     // backround class name
                        .arg(result->title())       // bab
                        .arg(result->id())          // entry id
                        .arg(result->snippet().simplified())    // text snippet
                        .arg(getBookName(result->bookId()))     // book name
                        .arg(result->page())        // page
                        .arg(result->part())        // part
                        .arg(result->bookId())      // book id
                        .arg(result->archive())     // book archive
                        .arg(result->score());      // score

    ui->webView->page()->mainFrame()->evaluateJavaScript(QString("addResult('%1');")
                                                         .arg(resultString));
    m_fetechProgressBar->setValue(m_fetechProgressBar->value()+1);
    /*
    QFile file("test.js");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QString("addResult('%1');").arg(resultString) << "\n";
    */

}

void MainWindow::showStatistic()
{
    try {
        IndexReader* r = IndexReader::open(qPrintable(m_currentIndex->path()));
//        int64_t ver = r->getCurrentVersion(qPrintable(m_currentIndex->path()));

        QString txt("<p dir=\"rtl\" style=\"padding:5px;font-weight:bold;\">");

        txt.append(trUtf8("اسم الفهرس: <strong style=\"color:green;\">%1</strong>" "<br />").arg(m_currentIndex->name()));
        txt.append(trUtf8("مسار الفهرس: <strong style=\"color:green;\">%1</strong>" "<br />").arg(m_currentIndex->path()));
        txt.append(trUtf8("مسار المكتبة الشاملة: <strong style=\"color:green;\">%1</strong>" "<br />").arg(m_currentIndex->shamelaPath()));
        //txt.append(tr("Max Docs: %1<br/>").arg(r->maxDoc()));
//        txt.append(tr("Current Version: %1<br/><br/>").arg(ver)) ;
        txt.append(trUtf8("عدد الصفحات: <strong style=\"color:green;\">%1</strong>" "<br />").arg(r->numDocs()));

        TermEnum* te = r->terms();
        int32_t nterms;

        bool writeToFile = false;

        if(writeToFile) {
            QFile logFile("terms.txt");
            if(logFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
                QTextStream log(&logFile);
                for (nterms = 0; te->next() == true; nterms++) {
                    /* if(_wcsicmp(te->term()->field(), _T("bookid")) == 0) */
                    log << TCHAR_TO_QSTRING(te->term()->toString()) << "\n";
                }
            }
        } else {
            for (nterms = 0; te->next() == true; nterms++) {
                /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
            }
        }

        txt.append(trUtf8("عدد الكلمات: <strong style=\"color:green;\">%1</strong>" "<br />").arg(nterms));
        txt.append(trUtf8("حجم الفهرس: <strong style=\"color:green;\">%1</strong>" "<br />").arg(getIndexSize()));
        txt.append(trUtf8("حجم الكتب المفهرسة: <strong style=\"color:green;\">%1</strong>" "<br />").arg(getBooksSize()));
        txt.append("</p>");

        QDialog *dialog = new QDialog(this);
        QVBoxLayout *layout = new QVBoxLayout();
        QTextBrowser *browser = new QTextBrowser(dialog);
        browser->setHtml(txt);
        QLabel *label = new QLabel(trUtf8(":معلومات حول الفهرس"), dialog);
        label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
        layout->addWidget(label);
        layout->addWidget(browser);

        dialog->setWindowTitle(windowTitle());
        dialog->setLayout(layout);
        dialog->show();

        _CLLDELETE(te);
        r->close();
        _CLLDELETE(r);
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error search", err.what());
    }
    catch(...) {
        qDebug() << "Error when opening : " << m_currentIndex->path();
    }

}

QString MainWindow::cleanString(QString str)
{
    str.replace(QRegExp("[\\x0627\\x0622\\x0623\\x0625]"), "[\\x0627\\x0622\\x0623\\x0625]");//ALEFs
    str.replace(QRegExp("[\\x0647\\x0629]"), "[\\x0647\\x0629]"); //TAH_MARBUTA -> HEH

    return str;
}

void MainWindow::resultsCount()
{
    this->statusBar()->showMessage(trUtf8("نتائج البحث %1")
                                   .arg(m_resultCount));
}

void MainWindow::displayResults()
{

    QString resultString;

    int start = m_searcher->currentPage() * m_resultParPage;
    int maxResult  =  (m_searcher->resultsCount() >= start+m_resultParPage)
                      ? start+m_resultParPage : m_searcher->resultsCount();
    bool whiteBG = true;
    int entryID;
    for(int i=start; i < maxResult;i++){

        int bookID = m_searcher->bookIdAt(i);
        int archive = m_searcher->ArchiveAt(i);
        int score = (int) (m_searcher->scoreAt(i) * 100.0);

        QString connName = (archive) ? QString("bid_%1").arg(archive) : QString("bid_%1_%2").arg(archive).arg(bookID);

        {
            QSqlDatabase bookDB;
            if(archive && QSqlDatabase::contains(connName)) {
                bookDB = QSqlDatabase::database(connName);
            } else {
                bookDB = QSqlDatabase::addDatabase("QODBC", connName);
                QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                                  .arg(buildFilePath(QString::number(bookID), archive));
                bookDB.setDatabaseName(mdbpath);
            }

            if (!bookDB.open())
                qDebug() << "Cannot open" << buildFilePath(QString::number(bookID), archive) << "database.";

            QSqlQuery bookQuery(bookDB);

            entryID = m_searcher->idAt(i);
            bookQuery.exec(QString("SELECT nass, page, part FROM %1 WHERE id = %2")
                             .arg((!archive) ? "book" : QString("b%1").arg(bookID))
                             .arg(entryID));
            if(bookQuery.first()){
                resultString.append(trUtf8("<div class=\"result\" class=\"%1\">"
                                           "<h3>%2</h3>"
                                           "<span class=\"progSpan\" style=\"width: %10px;\">"
                                           "<span class=\"progSpanContainre\"></span>"
                                           "</span>"
                                           "<a class=\"bookLink\" href=\"http://localhost/book.html?id=%3&bookid=%8&archive=%9\">%4</a>"
                                           "<p style=\"margin: 5px 0px 0px;\"> كتاب: <span class=\"bookName\">%5</span>"
                                           "<span style=\"float: left;\">الصفحة: <span style=\"margin-left: 7px;\">%6</span>  الجزء: <span>%7</span></span>"
                                           "</p></div>")
                                    .arg(whiteBG ? "whiteBG" : "grayBG") //BG color
                                    .arg(getTitleId(bookDB, entryID, archive, bookID)) //BAB
                                    .arg(entryID) // entry id
                                    .arg(hiText(abbreviate(bookQuery.value(0).toString(),320), m_searchQuery)) // TEXT
                                    .arg(getBookName(bookID)) // BOOK_NAME
                                    .arg(bookQuery.value(1).toString()) // PAGE
                                    .arg(bookQuery.value(2).toString()) // PART
                                    .arg(bookID)
                                    .arg(archive)
                                    .arg(score)
                                    );
                whiteBG = !whiteBG;
            }
        }
        if(!archive)
            QSqlDatabase::removeDatabase(connName);
    }

    setPageCount(m_searcher->currentPage()+1, m_searcher->pageCount());

    QString appPath(QString("file:///%1").arg(qApp->applicationDirPath()));

    ui->webView->setHtml(QString("<html><head><title></title>"
                                 "<link href=\"%2/data/default.css\"  rel=\"stylesheet\" type=\"text/css\" />"
                                 "</head>"
                                 "<body>%1</body>"
                                 "<script type=\"text/javascript\" src=\"%2/data/jquery-1.4.2.min.js\" />"
                                 "<script type=\"text/javascript\" src=\"%2/data/scripts.js\" />"
                                 "</html>")
                         .arg(resultString)
                         .arg(appPath));
}

void MainWindow::on_pushGoNext_clicked()
{
    m_searcher->nextPage();
}

void MainWindow::on_pushGoPrev_clicked()
{
    m_searcher->prevPage();
}

void MainWindow::on_pushGoFirst_clicked()
{
    m_searcher->firstPage();
}

void MainWindow::on_pushGoLast_clicked()
{
    m_searcher->lastPage();
}

QString MainWindow::hiText(const QString &text, const QString &strToHi)
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

QStringList MainWindow::buildRegExp(const QString &str)
{
QStringList strWords = str.split(QRegExp(trUtf8("[\\s;,.()\"'{}\\[\\]]")), QString::SkipEmptyParts);
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

void MainWindow::setPageCount(int current, int count)
{
    int rCount = qMax(1, count);
    int rCurrent = qMax(1, current);
    ui->label->setText(trUtf8("الصفحة %1 / %2")
                       .arg(rCount)
                       .arg(rCurrent));
    buttonStat(rCurrent, rCount);
}

void MainWindow::buttonStat(int currentPage, int pageCount)
{
    if(currentPage == 1) {
        ui->pushGoPrev->setEnabled(false);
        ui->pushGoFirst->setEnabled(false);

    } else {
        ui->pushGoPrev->setEnabled(true);
        ui->pushGoFirst->setEnabled(true);
    }

    if(currentPage == pageCount){
        ui->pushGoNext->setEnabled(false);
        ui->pushGoLast->setEnabled(false);
    } else {
        ui->pushGoNext->setEnabled(true);
        ui->pushGoLast->setEnabled(true);
    }


}

void MainWindow::on_lineQuery_returnPressed()
{
    startSearching();
}

bool MainWindow::openDB()
{
    m_dbIsOpen = false;

    m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaBook");
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                      .arg(m_currentIndex->shamelaDbPath());
    m_bookDB.setDatabaseName(mdbpath);

    if (!m_bookDB.open()) {
        QMessageBox::warning(0, "Error opening database", "Cannot open main.mdb database.");
        return false;
    }
    m_bookQuery = new QSqlQuery(m_bookDB);
    m_dbIsOpen = true;

    return true;
}

QString MainWindow::abbreviate(QString str, int size) {
        if (str.length() <= size-3)
                return str;
        str.simplified();
        int index = str.lastIndexOf(' ', size-3);
        if (index <= -1)
                return "";
        return str.left(index).append("...");
}

QString MainWindow::getIndexSize()
{
    QDir dir;
    dir.cd(m_currentIndex->path());
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QString sizeStr;
    qint64 size = 0;
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        size += fileInfo.size();
    }

    if(size < 1024)
        sizeStr = trUtf8("%1 بيت").arg(size);
    else if(1024 <= size && size < 1024*1024)
        sizeStr = trUtf8("%1 كيلو").arg(size/(1024.0), 4);
    else if( 1024*1024 <= size && size < 1024*1024*1024)
        sizeStr = trUtf8("%1 ميغا").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = trUtf8("%1 جيجا").arg(size/(1024.0*1024.0*1024.0), 4);

    return sizeStr;
}

QString MainWindow::getBooksSize()
{
    QString sizeStr;
    qint64 size = getDirSize(m_currentIndex->shamelaPath());

    if(size < 1024)
        sizeStr = trUtf8("%1 بيت").arg(size);
    else if(1024 <= size && size < 1024*1024)
        sizeStr = trUtf8("%1 كيلو").arg(size/(1024.0), 4);
    else if( 1024*1024 <= size && size < 1024*1024*1024)
        sizeStr = trUtf8("%1 ميغا").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = trUtf8("%1 جيجا").arg(size/(1024.0*1024.0*1024.0), 4);

    return sizeStr;
}

qint64 MainWindow::getDirSize(const QString &path)
{
    QFileInfo info(path);
    qint64 size=0;

    if(info.isDir()){
        QDir dir(path);
        foreach(QFileInfo fieInfo, dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
            if(fieInfo.isFile() && fieInfo.suffix() == "mdb")
                size += fieInfo.size();
            else if(fieInfo.isDir())
                size += getDirSize(fieInfo.absoluteFilePath());
        }
    }

    return size;
}

void MainWindow::doneIndexing(int indexingTime)
{
    try {
        IndexReader* r = IndexReader::open(qPrintable(m_currentIndex->path()));
        int64_t ver = r->getCurrentVersion(qPrintable(m_currentIndex->path()));

        QString txt;
        txt.append(tr("[+] Date: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:MM:ss")));
#ifdef GITVERSION
        txt.append(tr("[+] Git: %1 - Change number: %2\n").arg(GITVERSION).arg(GITCHANGENUMBER));
#endif
        txt.append(tr("[+] Statistics for \"%1\"\n").arg(m_currentIndex->path()));
        txt.append(tr("[+] Current Version: %1\n").arg(ver)) ;
        txt.append(tr("[+] Num Docs: %1\n").arg(r->numDocs()));

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
        }
        txt.append(tr("[+] Indexing took: %1 s\n").arg(indexingTime/1000.0));
        txt.append(tr("[+] Term count: %1\n").arg(nterms));
        txt.append(tr("[+] Index size: %1\n").arg(getIndexSize()));
        txt.append(tr("[+] Books size: %1\n").arg(getBooksSize()));
        txt.append("\n");
        _CLLDELETE(te);

        r->close();
        _CLLDELETE(r);

        QFile logFile("statistic.txt");
        if(logFile.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)) {
             QTextStream log(&logFile);
             log << txt;
        }
    }
    catch(...) {}

}

QString MainWindow::getTitleId(const QSqlDatabase &db, int pageID, int archive, int bookID)
{
    QSqlQuery m_bookQuery(db);
    m_bookQuery.exec(QString("SELECT TOP 1 tit FROM %1 WHERE id <= %2 ORDER BY id DESC")
                     .arg((!archive) ? "title" : QString("t%1").arg(bookID))
                     .arg(pageID));

    if(m_bookQuery.first())
        return m_bookQuery.value(0).toString();
    else {
        qDebug() << m_bookQuery.lastError().text();
        return QString();
    }

}

QString MainWindow::getBookName(int bookID)
{
    QSqlQuery m_bookQuery(QSqlDatabase::database("shamelaBook"));
    m_bookQuery.exec(QString("SELECT bk FROM 0bok WHERE bkid = %1").arg(bookID));

    if(m_bookQuery.first())
        return m_bookQuery.value(0).toString();
    else {
        qDebug() << m_bookQuery.lastError().text();
        return QString();
    }

}

void MainWindow::resultLinkClicked(const QUrl &url)
{
    return;
    int rid = url.queryItems().at(0).second.toInt();
    int bookID = url.queryItems().at(1).second.toInt();
    int archive = url.queryItems().at(2).second.toInt();

    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout= new QVBoxLayout(dialog);
    QTextBrowser *textBrowser = new QTextBrowser(0);
    QString text;
    layout->addWidget(textBrowser);
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "disBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(buildFilePath(QString::number(bookID), archive));
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open" << buildFilePath(QString::number(bookID), archive) << "database.";
        }
        QSqlQuery m_bookQuery(m_bookDB);

        m_bookQuery.exec(QString("SELECT page, part, nass FROM %1 WHERE id = %2")
                         .arg((!archive) ? "book" : QString("b%1").arg(bookID))
                         .arg(rid));
        if(m_bookQuery.first())
            text = m_bookQuery.value(2).toString();

        text.replace(QRegExp("[\\r\\n]"),"<br/>");
    }
    QSqlDatabase::removeDatabase("disBook");

    textBrowser->setHtml(hiText(text, m_searchQuery));
    textBrowser->setAlignment(Qt::AlignRight);

    dialog->setLayout(layout);
    dialog->resize(500,500);
    dialog->show();
}

void MainWindow::displayResultsOptions()
{
    QDialog *settingDialog =  new QDialog(this);
    QVBoxLayout *vLayout= new QVBoxLayout;
    QHBoxLayout *hLayout= new QHBoxLayout;
    QLabel *label = new QLabel(trUtf8(":عدد النتائج في كل صفحة"), settingDialog);
    QSpinBox *spinPage = new QSpinBox(settingDialog);
    QPushButton *pushDone = new QPushButton(trUtf8("حفظ"), settingDialog);

    spinPage->setMaximum(1000);
    spinPage->setMinimum(1);
    spinPage->setValue(m_resultParPage);
    hLayout->addWidget(spinPage);
    hLayout->addWidget(label);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(pushDone);
    settingDialog->setLayout(vLayout);
    settingDialog->show();

    connect(pushDone, SIGNAL(clicked()), settingDialog, SLOT(accept()));
    connect(spinPage, SIGNAL(valueChanged(int)), this, SLOT(setResultParPage(int)));
}

QString MainWindow::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(m_currentIndex->shamelaPath()).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(m_currentIndex->shamelaPath()).arg(archive);
}

void MainWindow::populateJavaScriptWindowObject()
{
    ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("MainWindow", this);
}

QString MainWindow::getPage(QString href)
{
    QUrl url(href);
    int rid = url.queryItems().at(0).second.toInt();
    int bookID = url.queryItems().at(1).second.toInt();
    int archive = url.queryItems().at(2).second.toInt();


    QString text;
    {
        QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "disBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(buildFilePath(QString::number(bookID), archive));
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open" << buildFilePath(QString::number(bookID), archive) << "database.";
        }
        QSqlQuery m_bookQuery(m_bookDB);

        m_bookQuery.exec(QString("SELECT id, nass FROM %1 WHERE id = %2")
                         .arg((!archive) ? "book" : QString("b%1").arg(bookID))
                         .arg(rid));
        if(m_bookQuery.first()) {
            text = m_bookQuery.value(1).toString();
            m_currentShownId = m_bookQuery.value(0).toInt();
        }
        text.replace(QRegExp("[\\r\\n]"),"<br/>");
    }
    QSqlDatabase::removeDatabase("disBook");

    return hiText(text, m_searchQuery);
}

QString MainWindow::formNextUrl(QString href)
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

QString MainWindow::formPrevUrl(QString href)
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

void MainWindow::updateNavgitionLinks(QString href)
{
    ui->webView->page()->mainFrame()->evaluateJavaScript(QString("updateLinks('%1', '%2');")
                                                         .arg(formNextUrl(href))
                                                         .arg(formPrevUrl(href)));
}
