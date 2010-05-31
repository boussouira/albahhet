#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->lineQuery->setText(trUtf8("الله ولي الذين امنوا"));
    setWindowTitle("CLucene Test");

    m_resultModel = new QSqlQueryModel(this);
    resultModel = new QStandardItemModel();
    resultCount = 0;
    dbIsOpen = false;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "cluceneTest", "E-Shamela");
    ui->lineBook->setText(settings.value("db").toString());

    connect(ui->pushIndex, SIGNAL(clicked()), this, SLOT(startIndexing()));
    connect(ui->pushSearch, SIGNAL(clicked()), this, SLOT(startSearching()));
    connect(ui->pushStatstic, SIGNAL(clicked()), this, SLOT(showStatistic()));
    connect(ui->pushRCount, SIGNAL(clicked()), this, SLOT(resultsCount()));
}

MainWindow::~MainWindow()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "cluceneTest", "E-Shamela");
    settings.setValue("db", ui->lineBook->text());
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
    if(!dbIsOpen)
		openDB();
    try {
        IndexWriter* writer = NULL;
        QDir dir;
//        SimpleAnalyzer* an = _CLNEW SimpleAnalyzer();
//        lucene::analysis::standard::StandardAnalyzer an;
        ArabicAnalyzer analyzer;
        if(!dir.exists(INDEX_PATH))
            dir.mkdir(INDEX_PATH);
        if ( IndexReader::indexExists(INDEX_PATH) ){
            if ( IndexReader::isLocked(INDEX_PATH) ){
                printf("Index was locked... unlocking it.\n");
                IndexReader::unlock(INDEX_PATH);
            }

            writer = _CLNEW IndexWriter( INDEX_PATH, &analyzer, true);
        }else{
            writer = _CLNEW IndexWriter( INDEX_PATH ,&analyzer, true);
        }
        writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);

        QTime time;
        time.start();

        indexDocs(writer);
        writer->optimize();
        writer->close();
        _CLDELETE(writer)

                statusBar()->showMessage(trUtf8("تمت الفهرسة خلال %1 "SECONDE_AR).arg(miTOsec(time.elapsed())));
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error when Indexing", err.what());
    }
}

void MainWindow::startSearching()
{
    resultModel->clear();
    if(!dbIsOpen)
        openDB();
    try {
        //Searcher searcher(INDEX_PATH);
//        standard::StandardAnalyzer analyzer;
        ArabicAnalyzer analyzer;

        QString queryWord = ui->lineQuery->text();

        if(ui->checkBox->isChecked()) {
            queryWord.replace(" ", " AND ");
        }
        const TCHAR* buf;
        IndexSearcher s(INDEX_PATH);

        // Start building the query
        Query* q = QueryParser::parse(QSTRING_TO_TCHAR(queryWord),_T("text"),&analyzer);

        buf = q->toString(_T("text"));
		qDebug() << "Searching for: " << TCHAR_TO_QSTRING(buf);
        //    _CLDELETE_CARRAY(buf);

        QTime time;
        QList<int> resultList;
        QList<float_t> scoreList;

        time.start();
        Hits* h = s.search(q);
        int timeSearch = time.restart();

        resultCount = h->length();
        for (int i=0;i<resultCount;i++ ){
            Document* doc = &h->doc(i);
            scoreList.append(h->score(i));
            resultList.append(FIELD_TO_INT("id", doc));

            //delete doc;
        }
        m_resultStruct.results = resultList;
        m_resultStruct.scoring = scoreList;
        m_resultStruct.page = 0;
        displayResults(/*ResultStrc*/);

        this->statusBar()->showMessage(trUtf8("تم البحث خلال %1 "SECONDE_AR".  "
                                               "نتائج البحث %2")
                                       .arg(miTOsec(timeSearch))
                                       .arg(resultCount));
        _CLDELETE(h);
        _CLDELETE(q);
        s.close();
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
        txt.append(tr("Statistics for %1\n").arg(INDEX_PATH));
        txt.append(tr("Max Docs: %1\n").arg(r->maxDoc()));
        txt.append(tr("Num Docs: %1\n").arg(r->numDocs()));
        txt.append(tr("Current Version: %1\n").arg(ver)) ;

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
        }
        txt.append(tr("Term count: %1").arg(nterms));
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

void MainWindow::indexDocs(IndexWriter* writer)
{
    m_bookQuery->exec(QString(SQL_QUERY_INDEX).arg(mainName));
    while(m_bookQuery->next())
    {
        Document* doc = FileDocument(m_bookQuery->value(0).toString(),
                                     m_bookQuery->value(1).toString());
        writer->addDocument( doc );
        _CLDELETE(doc);
    }
}

Document* MainWindow::FileDocument(const QString &id, const QString &ayaText)
{
    // make a new, empty document
    Document* doc = _CLNEW Document();

    doc->add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(id) ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
    doc->add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(ayaText), Field::STORE_NO | Field::INDEX_TOKENIZED) );

    return doc;
}

QString MainWindow::cleanString(QString str)
{
    str.remove(QRegExp(trUtf8("[ًٌٍَُِّْ]")));
/*
    str.replace(ALEF_WITH_HAMZA_ABOVE, ALEF);
    str.replace(ALEF_WITH_HAMZA_BELOW, ALEF);
    str.replace(ALEF_WITH_MADDA_ABOVE, ALEF);
    str.replace(HAMZA_ABOVE_ALEF, ALEF);
    str.replace(MARBUTA, HEH);
*/
    str.remove(QRegExp("\\[([^\\[]+)\\]"));
    str.remove(QRegExp("\\[co\\]([^\\[]+)\\[/co\\]"));
    str.remove(QRegExp("\\(([a-zA-Z0-9]+]+)\\)"));
    str.remove(QRegExp("\\[([a-zA-Z0-9]+)\\]"));
    return str;
}

void MainWindow::resultsCount()
{
    this->statusBar()->showMessage(trUtf8("نتائج البحث %1")
                                   .arg(resultCount));
}

void MainWindow::displayResults(/*result &pResult*/)
{
    QStringList headersList;
    QList<QStandardItem *> listItems;

    headersList << trUtf8("النص") << trUtf8("الصفحة") << trUtf8("الجزء") << trUtf8("التطابق");
    resultModel->clear();

    int start = m_resultStruct.page * 10;
    int maxResult  =  (m_resultStruct.results.count() >= start+10) ? start+10 : m_resultStruct.results.count();

    for(int i=start; i < maxResult;i++){
        m_bookQuery->exec(QString(SQL_QUERY).arg(mainName).arg(m_resultStruct.results.at(i)));
        if(m_bookQuery->first()){
            listItems.append(new QStandardItem(abbreviate(m_bookQuery->value(2).toString(),120)));
            listItems.append(new QStandardItem(m_bookQuery->value(1).toString()));
            listItems.append(new QStandardItem(m_bookQuery->value(0).toString()));
            listItems.append(new QStandardItem(QString::number(m_resultStruct.scoring.at(i), 'f', 4)));
            resultModel->appendRow(listItems);
            listItems.clear();
        }
    }
    resultModel->setHorizontalHeaderLabels(headersList);
    resultModel->setVerticalHeaderLabels(makeVLabels(start, maxResult));
    ui->tableView->setModel(resultModel);
    setPageCount(m_resultStruct.page+1, _toBint(m_resultStruct.scoring.count()/10.0));
    resizeTable();
}

void MainWindow::on_pushGoNext_clicked()
{
    m_resultStruct.page++;
    displayResults();
}

void MainWindow::on_pushGoPrev_clicked()
{
    m_resultStruct.page--;
    displayResults();
}

void MainWindow::on_pushGoFirst_clicked()
{
    m_resultStruct.page = 0;
    displayResults();
}

void MainWindow::on_pushGoLast_clicked()
{
    m_resultStruct.page = _toBint(m_resultStruct.scoring.count()/10.0)-1;
    displayResults();
}

void MainWindow::on_tableView_doubleClicked(QModelIndex index)
{
    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout= new QVBoxLayout(dialog);
    QTextBrowser *textBrowser = new QTextBrowser(0);
    QString text;
    layout->addWidget(textBrowser);

    int rid = (m_resultStruct.page*10)+index.row();
    rid = m_resultStruct.results.at(rid);

    m_bookQuery->exec(QString(SQL_QUERY).arg(mainName).arg(rid));
    if(m_bookQuery->first())
        text = m_bookQuery->value(2).toString();

    text.replace(QRegExp("[\\r|\\n]"),"<br/>");
    text = hiText(text, ui->lineQuery->text());

    textBrowser->setHtml(text);
    textBrowser->setAlignment(Qt::AlignRight);

    dialog->setLayout(layout);
    dialog->resize(500,500);
    dialog->show();
}

QString MainWindow::hiText(const QString &text, const QString &strToHi)
{
    QStringList regExpStr = buildRegExp(strToHi);
    QString finlStr  = text;
    QTextCharFormat txtForm;

    txtForm.setBackground(Qt::yellow);
    foreach(QString regExp, regExpStr)
        finlStr = finlStr.replace(QRegExp(regExp), "<span style=\"background-color:yellow\">\\1</span>");

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
        regExpStr.append(opPar);

        for (int i=0; i< word.size();i++) {
            regExpStr.append(word.at(i));
            regExpStr.append(trUtf8("[ًٌٍَُِّْ]*"));
        }

        regExpStr.append(clPar);
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

QStringList MainWindow::makeVLabels(int start, int end)
{
    QStringList labls;
    for(int i=start+1; i<=end;i++)
        labls.append(QString::number(i));
    return labls;
}

void MainWindow::resizeTable()
{
    ui->tableView->resizeColumnToContents(1);
    ui->tableView->resizeColumnToContents(2);
    ui->tableView->resizeColumnToContents(3);
    ui->tableView->setColumnWidth(0,ui->tableView->width()-
                                  (ui->tableView->columnWidth(1)+
                                   ui->tableView->columnWidth(2)+
                                   ui->tableView->columnWidth(3))-40);
}

void MainWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 trUtf8("اختر الكتاب المراد البحث فيه"), "",
                                 tr("Access (*.bok *.mdb);;SQLITE (*.db)"));
    if(!path.isEmpty()) {
        ui->lineBook->setText(path);
        if(QSqlDatabase::contains("shamelaBook"))
            QSqlDatabase::removeDatabase("shamelaBook");
        openDB();
        int rep = QMessageBox::question(this,
                                        trUtf8("فهرسة الكتاب"),
                                        trUtf8("هل تريد فهرسة الكتاب ؟"),
                                        QMessageBox::Yes|QMessageBox::No);
        if(rep==QMessageBox::Yes)
            startIndexing();
    }
}

void MainWindow::openDB()
{
    dbIsOpen = false;
    QString book = (ui->lineBook->text());
    QString sqlite_book(book);

    if(book.contains(QRegExp("\\.(mdb|bok)$"))) {
#ifdef Q_OS_WIN32
        m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaBook");
        QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(book);
        m_bookDB.setDatabaseName(mdbpath);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open MDB database.";
        }
        m_bookQuery = new QSqlQuery(m_bookDB);
        dbIsOpen = true;
#else
        qDebug() << "Need to convert the database.";
        sqlite_book.append(".sqlite");
        if(!QFile::exists(sqlite_book)) {
            MdbConverter *mdb = new MdbConverter;
            mdb->exportFromMdb(qPrintable(book), qPrintable(sqlite_book));
        }
#endif
    }

    if(!dbIsOpen) {
        m_bookDB = QSqlDatabase::addDatabase("QSQLITE", "shamelaBook");
        m_bookDB.setDatabaseName(sqlite_book);

        if (!m_bookDB.open()) {
            qDebug() << "Cannot open database.";
        }
        m_bookQuery = new QSqlQuery(m_bookDB);
        dbIsOpen = true;
    }

    QStringList tables = m_bookDB.tables();
    foreach(QString ta, tables) {
        if( ta.contains(QRegExp("(t[0-9]+|title)")) ){
            tocName = ta;
        } else if( ta.contains(QRegExp("(b[0-9]+|book)")) ){
            mainName = ta;
        }
    }
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
