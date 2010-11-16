#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("CLucene Test");

    m_resultModel = new QStandardItemModel();
    m_results = new Results();
    m_resultCount = 0;
    m_resultParPage = 10;
    m_dbIsOpen = false;

    m_colors.append("#FFFF63");
    m_colors.append("#A5FFFF");
    m_colors.append("#FF9A9C");
    m_colors.append("#9CFF9C");
    m_colors.append("#EF86FB");

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "cluceneTest", "cluceneTestX");
    m_bookPath = settings.value("db").toString();
    if(!m_bookPath.endsWith('/'))
        m_bookPath.append('/');
    m_searchQuery = settings.value("lastQuery").toString();
    m_resultParPage = settings.value("resultPeerPage", m_resultParPage).toInt();
    ui->lineBook->setText(m_bookPath);
    ui->lineQuery->setText(m_searchQuery);

    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    ui->lineQuery->setLayoutDirection(Qt::LeftToRight);
    ui->lineQuery->setLayoutDirection(Qt::RightToLeft);

    ui->pushStatstic->hide();

    connect(ui->pushIndex, SIGNAL(clicked()), this, SLOT(startIndexing()));
    connect(ui->pushSearch, SIGNAL(clicked()), this, SLOT(startSearching()));
    connect(ui->pushStatstic, SIGNAL(clicked()), this, SLOT(showStatistic()));
    connect(ui->pushRCount, SIGNAL(clicked()), this, SLOT(resultsCount()));
    connect(ui->webView, SIGNAL(linkClicked(QUrl)), this, SLOT(resultLinkClicked(QUrl)));
}

MainWindow::~MainWindow()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "cluceneTest", "cluceneTestX");
    settings.setValue("db", m_bookPath);
    settings.setValue("lastQuery", ui->lineQuery->text());
    settings.setValue("resultPeerPage", m_resultParPage);
    delete ui;
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

void MainWindow::startIndexing()
{
    if(!m_dbIsOpen)
        openDB();
    {
        QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndex");
        indexDB.setDatabaseName("book_index.db");
        if(!indexDB.open())
            qDebug("Error opning index db");
        QSqlQuery *inexQuery = new QSqlQuery(indexDB);
        inexQuery->exec("DROP TABLE IF EXISTS books");
        inexQuery->exec("CREATE TABLE IF NOT EXISTS books (id INTEGER PRIMARY KEY, bookName TEXT, "
                        "shamelaID INTEGER, filePath TEXT, authorId INTEGER, authorName TEXT, "
                        "fileSize INTEGER, cat INTEGER, archive INTEGER, titleTable TEXT, bookTable TEXT)");


        indexDB.transaction();
        if(m_bookQuery->exec("SELECT Bk, bkid, auth, authno, Archive FROM 0bok"))
            while(m_bookQuery->next()) {
            int archive = m_bookQuery->value(4).toInt();
            if(!inexQuery->exec(QString("INSERT INTO books VALUES "
                                        "(NULL, '%1', %2, '%3', %4, '%5', '', '', %6, '%7', '%8')")
                .arg(m_bookQuery->value(0).toString())
                .arg(m_bookQuery->value(1).toString())
                .arg(buildFilePath(m_bookQuery->value(1).toString(), archive))
                .arg(m_bookQuery->value(3).toString())
                .arg(m_bookQuery->value(2).toString())
                .arg(archive)
                .arg((!archive) ? "title" : QString("t%1").arg(m_bookQuery->value(1).toInt()))
                .arg((!archive) ? "book" : QString("b%1").arg(m_bookQuery->value(1).toInt()))))
                qDebug()<< "ERROR:" << inexQuery->lastError().text();
        }
        indexDB.commit();

        delete inexQuery;
        indexDB.close();
    }
    QSqlDatabase::removeDatabase("bookIndex");
    IndexingDialg *indexDial = new IndexingDialg(this);
    indexDial->show();
}

void MainWindow::startSearching()
{
    m_resultModel->clear();
    if(!m_dbIsOpen)
        openDB();
    try {
        ArabicAnalyzer analyzer;

        m_searchQuery = ui->lineQuery->text();
        QString queryWord = m_searchQuery;

        if(ui->checkBox->isChecked()) {
            queryWord = queryWord.trimmed().replace(" ", " AND ");
        } else {
            queryWord.replace(QRegExp(trUtf8("ـفق")), "(");
            queryWord.replace(QRegExp(trUtf8("ـغق")), ")");
            queryWord.replace(QRegExp(trUtf8("ـ[أا]و")), "OR");
            queryWord.replace(QRegExp(trUtf8("ـو")), "AND");
            queryWord.replace(QRegExp(trUtf8("ـبدون")), "NOT");
            queryWord.replace(trUtf8("؟"), "?");
        }

        IndexSearcher *searcher = new IndexSearcher(INDEX_PATH);

        // Start building the query
        QueryParser *queryPareser = new QueryParser(_T("text"),&analyzer);
        queryPareser->setAllowLeadingWildcard(true);

        Query* q = queryPareser->parse(QSTRING_TO_TCHAR(queryWord));
//        qDebug() << "Search: " << TCHAR_TO_QSTRING(q->toString(_T("text")));
//        qDebug() << "Query : " << queryWord;

        QTime time;

        time.start();
        m_results->setHits(searcher->search(q));
        int timeSearch = time.elapsed();

        m_resultCount = m_results->resultsCount();
        m_results->setPageCount(_toBInt((m_results->resultsCount()/(double)m_resultParPage)));
        m_results->setCurrentPage(0);
        displayResults();

        this->statusBar()->showMessage(trUtf8("تم البحث خلال %1 "SECONDE_AR".  "
                                               "نتائج البحث %2")
                                       .arg(miTOsec(timeSearch))
                                       .arg(m_resultCount));
//        _CLDELETE(hits);
//        _CLDELETE(q);
//        searcher->close();
    }

    catch(CLuceneError &tmp) {
        QMessageBox::warning(0, "Error search", tmp.what());
    }

    catch(...) {
        qDebug() << "Error when searching at : " << INDEX_PATH;
    }

}

void MainWindow::showStatistic()
{
    try {
        IndexReader* r = IndexReader::open(INDEX_PATH);
        int64_t ver = r->getCurrentVersion(INDEX_PATH);

        QString txt;
        txt.append(tr("Statistics for <strong>%1</strong><br/>").arg(INDEX_PATH));
        //txt.append(tr("Max Docs: %1<br/>").arg(r->maxDoc()));
        txt.append(tr("Current Version: %1<br/><br/>").arg(ver)) ;
        txt.append(tr("Num Docs: %1<br/>").arg(r->numDocs()));

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
        }
        txt.append(tr("Term count: %1<br/>").arg(nterms));
        txt.append(tr("Index size: %1<br/>").arg(getIndexSize()));
        txt.append(tr("Book size: %1").arg(getBookSize()));
        _CLDELETE(te);

        r->close();
        _CLDELETE(r);
        QMessageBox::information(0, "Statistics", txt);
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error search", err.what());
    }
    catch(...) {
        qDebug() << "Error when opening : " << INDEX_PATH;
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

void MainWindow::displayResults(/*result &pResult*/)
{

    QString resultString;

    int start = m_results->currentPage() * m_resultParPage;
    int maxResult  =  (m_results->resultsCount() >= start+m_resultParPage)
                      ? start+m_resultParPage : m_results->resultsCount();
    bool whiteBG = true;
    int entryID;
    for(int i=start; i < maxResult;i++){
        {
            int bookID = m_results->bookIdAt(i);
            int archive = m_results->ArchiveAt(i);
            QSqlDatabase m_bookDB = QSqlDatabase::addDatabase("QODBC", "resultBook");
            QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                              .arg(buildFilePath(QString::number(bookID), archive));
            m_bookDB.setDatabaseName(mdbpath);

            if (!m_bookDB.open()) {
                qDebug() << "Cannot open" << buildFilePath(QString::number(bookID), archive) << "database.";
            }
            QSqlQuery m_bookQuery(m_bookDB);

            entryID = m_results->idAt(i);
            m_bookQuery.exec(QString("SELECT nass, page, part FROM %1 WHERE id = %2")
                             .arg((!archive) ? "book" : QString("b%1").arg(bookID))
                             .arg(entryID));
            if(m_bookQuery.first()){
                resultString.append(trUtf8("<div class=\"result\" class=\"%1\">"
                                           "<h3>%2</h3>"
                                           "<a href=\"http://localhost/book.html?id=%3&bookid=%8&archive=%9\">%4</a>"
                                           "<p style=\"margin: 5px 0px 0px;\"> كتاب: <span class=\"bookName\">%5(%9)</span>"
                                           "<span style=\"float: left;\">الصفحة: <span style=\"margin-left: 7px;\">%6</span>  الجزء: <span>%7</span></span>"
                                           "</p></div>")
                                    .arg(whiteBG ? "whiteBG" : "grayBG") //BG color
                                    .arg(getTitleId(entryID, archive, bookID)) //BAB
                                    .arg(entryID) // entry id
                                    .arg(hiText(abbreviate(m_bookQuery.value(0).toString(),320), m_searchQuery)) // TEXT
                                    .arg(getBookName(bookID)) // BOOK_NAME
                                    .arg(m_bookQuery.value(1).toString()) // PAGE
                                    .arg(m_bookQuery.value(2).toString()) // PART
                                    .arg(bookID)
                                    .arg(archive)
                                    );
                whiteBG = !whiteBG;
            }
        }
        QSqlDatabase::removeDatabase("resultBook");
    }

    setPageCount(m_results->currentPage()+1, m_results->pageCount());
    ui->webView->setHtml("<html><head><style>"
                         "a{text-decoration: none;color:#000000;} a:hover {color:blue}"
                         ".result{margin:0px;padding:4px;border-bottom:1px solid black;} "
                         ".whiteBG{background-color:#FFFFFF;} .grayBG{background-color:#EFEFEF;} "
                         ".result h3{margin:0px 0px 5px;font-size:18px;color:rgb(0, 68, 255);} "
                         ".result a{margin:0px;padding:0px 5px;} "
                         ".bookName{margin-left:7px;color:green;font-weight:bold}"
                         "</style></head>"
                         "<body style=\"direction: rtl; margin: 0pt; padding: 2px;\">"+resultString+"</body></html>");

}

void MainWindow::on_pushGoNext_clicked()
{
    m_results->setCurrentPage(m_results->currentPage()+1);
    displayResults();
}

void MainWindow::on_pushGoPrev_clicked()
{
    m_results->setCurrentPage(m_results->currentPage()-1);
    displayResults();
}

void MainWindow::on_pushGoFirst_clicked()
{
    m_results->setCurrentPage(0);
    displayResults();
}

void MainWindow::on_pushGoLast_clicked()
{
    m_results->setCurrentPage(m_results->pageCount()-1);
    displayResults();
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
    QStringList strWords = str.split(" ",  QString::SkipEmptyParts);
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
//    qDebug() << regExpList;
    return regExpList;
}

void MainWindow::setPageCount(int current, int count)
{
    int rCount = _atLeastOne(count);
    int rCurrent = _atLeastOne(current);
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

void MainWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 trUtf8("اختر مجلد المكتبة الشاملة"));
    if(!path.isEmpty()) {
        m_bookPath = path;
        if(!m_bookPath.endsWith('/'))
            m_bookPath.append('/');
        ui->lineBook->setText(m_bookPath);
        if(QSqlDatabase::contains("shamelaBooks"))
            QSqlDatabase::removeDatabase("shamelaBooks");
        if(openDB()){
            int rep = QMessageBox::question(this,
                                            trUtf8("فهرسة المكتبة"),
                                            trUtf8("هل تريد فهرسة المكتبة ؟"),
                                            QMessageBox::Yes|QMessageBox::No);
            if(rep==QMessageBox::Yes)
                startIndexing();
        }
    }
}

bool MainWindow::openDB()
{
    m_dbIsOpen = false;
    QString book = m_bookPath;
    book.append("Files/main.mdb");

    if(!QFile::exists(book)) {
        QMessageBox::warning(this,
                             trUtf8("خطأ في اختيار مجلد الشاملة"),
                             trUtf8("المرجوا اختيار مجلد المكتبة الشاملة"));
        return false;
    }
    m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaBook");
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
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
    dir.cd(INDEX_PATH);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QString sizeStr;
    int size = 0;
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        size += fileInfo.size();
    }

    if(size <= 1024)
        sizeStr = tr("%1 B").arg(size);
    else if(1024 <= size && size <= 1024*1024)
        sizeStr = tr("%1 KB").arg(size/(1024.0), 4);
    else if(size >= 1024*1024)
        sizeStr = tr("%1 MB").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = tr("%1 ??").arg(size);

    QFileInfo fileInfo(m_bookPath);
    int bookSize = fileInfo.size();

    sizeStr.append(tr(" (%1%)").arg(((double)size/(double)bookSize)*100.0, 2));
    return sizeStr;
}

QString MainWindow::getBookSize()
{
    QString sizeStr;
    QFileInfo fileInfo(m_bookPath);
    int size = fileInfo.size();

    if(size <= 1024)
        sizeStr = tr("%1 B").arg(size);
    else if(1024 <= size && size <= 1024*1024)
        sizeStr = tr("%1 KB").arg(size/(1024.0), 4);
    else if(size >= 1024*1024)
        sizeStr = tr("%1 MB").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = tr("%1 ??").arg(size);

    return sizeStr;
}

void MainWindow::writeLog(int indexingTime)
{
    try {
        IndexReader* r = IndexReader::open(INDEX_PATH);
        int64_t ver = r->getCurrentVersion(INDEX_PATH);

        QString txt;
        txt.append(tr("[+] Date: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:MM:ss")));
#ifdef GITVERSION
        txt.append(tr("[+] Git: %1 - Change number: %2\n").arg(GITVERSION).arg(GITCHANGENUMBER));
#endif
        txt.append(tr("[+] Statistics for \"%1\"\n").arg(INDEX_PATH));
        txt.append(tr("[+] Book: \"%1\"\n").arg(m_bookPath));
        txt.append(tr("[+] Current Version: %1\n").arg(ver)) ;
        txt.append(tr("[+] Num Docs: %1\n").arg(r->numDocs()));

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
        }
        txt.append(tr("[+] Indexing took: %1 s\n").arg(miTOsec(indexingTime)));
        txt.append(tr("[+] Term count: %1\n").arg(nterms));
        txt.append(tr("[+] Index size: %1\n").arg(getIndexSize()));
        txt.append(tr("[+] Book size: %1\n").arg(getBookSize()));
        txt.append("===========================================\n\n");
        _CLDELETE(te);

        r->close();
        _CLDELETE(r);

        QFile logFile("statistic.txt");
        if(logFile.open(QIODevice::Append)) {
             QTextStream log(&logFile);
             log << txt;
        }

    }
    catch(...) {}

}

QString MainWindow::getTitleId(int pageID, int archive, int bookID)
{
    QSqlQuery m_bookQuery(QSqlDatabase::database("resultBook"));
    m_bookQuery.exec(QString("SELECT TOP 1 tit FROM %1 WHERE id <= %2 ORDER BY id DESC")
                     .arg((!archive) ? "title" : QString("t%1").arg(bookID))
                     .arg(pageID));

    if(m_bookQuery.first())
        return m_bookQuery.value(0).toString();
    else {
        qDebug() << m_bookQuery.lastError().text();
        return m_bookName;
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
        return m_bookName;
    }

}

void MainWindow::resultLinkClicked(const QUrl &url)
{
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

void MainWindow::on_pushButton_2_clicked()
{
    QDialog *settingDialog =  new QDialog(this);
    QVBoxLayout *vLayout= new QVBoxLayout(settingDialog);
    QHBoxLayout *hLayout= new QHBoxLayout(settingDialog);
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
        return QString("%1Books/%2/%3.mdb").arg(m_bookPath).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1Books/Archive/%2.mdb").arg(m_bookPath).arg(archive);
}
