#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifndef QURAN_TEXT
    #define INDEX_PATH  "quran_index"
    #define DATABASE_PATH   "quran.db"
    #define SQL_QUERY   "SELECT QuranSowar.SoraName, QuranText.soraNumber, QuranText.ayaNumber, QuranText.ayaText " \
                        "FROM QuranText " \
                        "LEFT JOIN QuranSowar " \
                        "ON QuranSowar.id = QuranText.soraNumber " \
                        "WHERE QuranText.id = %1"
    #define SQL_QUERY_INDEX "SELECT QuranText.id, QuranText.ayaText, QuranText.ayaNumber, " \
                            "QuranText.pageNumber, QuranText.soraNumber " \
                            "FROM QuranText " \
                            "ORDER BY QuranText.id "
#else
    #define INDEX_PATH  "t-tabari"
    #define DATABASE_PATH   "t-tabari.db"
    #define SQL_QUERY   "SELECT sora, aya, text FROM tafesirText WHERE id = %1"
    #define SQL_QUERY_INDEX "SELECT id, text FROM tafesirText ORDER BY id "
*/
    #define INDEX_PATH  "bokari"
    #define DATABASE_PATH   INDEX_PATH".db"
    #define SQL_QUERY   "SELECT page, part, nass FROM book WHERE id = %1"
    #define SQL_QUERY_INDEX "SELECT id, nass FROM book ORDER BY id "

#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    {
    // letters used in Quran text
     #define HAMZA QChar(0x0621)
     #define ALEF_WITH_MADDA_ABOVE QChar(0x0622)
     #define ALEF_WITH_HAMZA_ABOVE QChar(0x0623)
     #define WAW_WITH_HAMZA_ABOVE QChar(0x0624)
     #define ALEF_WITH_HAMZA_BELOW QChar(0x0625)
     #define YEH_WITH_HAMZA QChar(0x0626)
     #define ALEF QChar(0x0627)
     #define BEH QChar(0x0628)
     #define MARBUTA QChar(0x0629)
     #define TEH QChar(0x062A)
     #define THEH QChar(0x062B)
     #define JEMM QChar(0x062C)
     #define HAH QChar(0x062D)
     #define KHAH QChar(0x062E)
     #define DAL QChar(0x062F)
     #define THAL QChar(0x0630)
     #define REH QChar(0x0631)
     #define ZAIN QChar(0x0632)
     #define SEEN QChar(0x0633)
     #define SHEEN QChar(0x0634)
     #define SAD QChar(0x0635)
     #define DAD QChar(0x0636)
     #define TAH QChar(0x0637)
     #define ZAH QChar(0x0638)
     #define AIN QChar(0x0639)
     #define GHAIN QChar(0x063A)
     #define TATWEEL QChar(0x0640)
     #define FEH QChar(0x0641)
     #define QAF QChar(0x0642)
     #define KAF QChar(0x0643)
     #define LAM QChar(0x0644)
     #define MEEM QChar(0x0645)
     #define NOON QChar(0x0646)
     #define HEH QChar(0x0647)
     #define WAW QChar(0x0648)
     #define ALEF_MAKSURA QChar(0x0649)
     #define YEH QChar(0x064A)
     #define FATHATAN QChar(0x064B)
     #define DAMMATAN QChar(0x064C)
     #define KASRATAN QChar(0x064D)
     #define FATHA QChar(0x064E)
     #define DAMMA QChar(0x064F)
     #define KASRA QChar(0x0650)
     #define SHADDA QChar(0x0651)
     #define SUKUN QChar(0x0652)
     #define MADDA QChar(0x0653)
     #define HAMZA_ABOVE QChar(0x0654)
     #define HAMZA_BELOW QChar(0x0655)
     #define SMALL_ALEF QChar(0x065F)
     #define SUPERSCRIPT_ALEF QChar(0x0670)
     #define ALEF_WASLA QChar(0x0671)
     #define HIGH_SALA QChar(0x06D6)
     #define HIGH_GHALA QChar(0x06D7)
     #define HIGH_MEEM_INITIAL_FORM QChar(0x06D8)
     #define HIGH_LA QChar(0x06D9)
     #define HIGH_JEMM QChar(0x06DA)
     #define HIGH_THREE_DOT QChar(0x06DB)
     #define HIGH_SEEN QChar(0x06DC)
     #define RUB_EL_HIZB QChar(0x06DE)
     #define HIGH_ROUNDED_ZERO QChar(0x06DF)
     #define HIGH_UPRIGHT_ZERO QChar(0x06E0)
     #define HIGH_MEEM QChar(0x06E2)
     #define LOW_SEEN QChar(0x06E3)
     #define SMALL_WAW QChar(0x06E5)
     #define SMALL_YEH QChar(0x06E6)
     #define HIGH_NOON QChar(0x06E8)
     #define SAJDAH QChar(0x06E9)
     #define LOW_STOP QChar(0x06EA)
     #define HIGH_STOP QChar(0x06EB)
     #define HIGH_STOP_FILLED QChar(0x06EC)
     #define LOW_MEEM QChar(0x06ED)
     #define HAMZA_ABOVE_ALEF QChar(0x0675)
     #define DOTLESS_BEH QChar(0x066E)
     #define HIGH_YEH QChar(0x06E7)
     #define ZWNJ QChar(0x200C)
     #define NBSP QChar(0x00A0)
     #define NNBSP QChar(0x202F)

     // other letters
     #define ARABIC_COMMA QChar(0x060C)
     #define ARABIC_SEMICOLON QChar(0x061B)

     #define FARSI_YEH QChar(0x06CC)
     #define FARSI_HIGH_HAMZA QChar(0x0674)
     #define FARSI_KEHEH QChar(0x06A9)
     #define SWASH_KAF QChar(0x06AA)
     #define YEH_BARREE QChar(0x06D2)
}
//    qDebug() << letterMap;
    ui->lineQuery->setText(trUtf8("الله ولي الذين امنوا"));
    m_quranDB = QSqlDatabase::addDatabase("QSQLITE", "QuranTextDB");
    m_quranDB.setDatabaseName(DATABASE_PATH);

    if (!m_quranDB.open()) {
        qDebug() << "Cannot open database.";
    }
    m_quranQuery = new QSqlQuery(m_quranDB);
    m_resultModel = new QSqlQueryModel(this);
    resultModel = new QStandardItemModel();
    resultCount = 0;

    connect(ui->pushIndex, SIGNAL(clicked()), this, SLOT(startIndexing()));
    connect(ui->pushSearch, SIGNAL(clicked()), this, SLOT(startSearching()));
    connect(ui->pushStatstic, SIGNAL(clicked()), this, SLOT(showStatistic()));
    connect(ui->pushRCount, SIGNAL(clicked()), this, SLOT(resultsCount()));

    //    this->showStatistic();
}

MainWindow::~MainWindow()
{
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
        m_resultStruct.offest = 0;
        displayResults(/*ResultStrc*/);

        QStringList headersList;
        headersList << trUtf8("سورة") << trUtf8("الآية") << trUtf8("النص") << trUtf8("التطابق");
        resultModel->setHorizontalHeaderLabels(headersList);

        ui->tableView->setModel(resultModel);
        //ui->tableView->resizeColumnsToContents();
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
            /* qDebug() << QString::fromWCharArray(te->term()->text()); */
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
    m_quranQuery->exec(SQL_QUERY_INDEX);
    while(m_quranQuery->next())
    {
        Document* doc = FileDocument(m_quranQuery->value(0).toString(),
                                     m_quranQuery->value(1).toString());
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
    QList<QStandardItem *> listItems;
    int start = m_resultStruct.offest + 10;
    int maxResult  =  (m_resultStruct.results.count() >= start) ? start : m_resultStruct.results.count();

    for(int i=m_resultStruct.offest; i < maxResult;i++){
        m_quranQuery->exec(QString(SQL_QUERY).arg(m_resultStruct.results.at(i)));
        if(m_quranQuery->first()){
            listItems.append(new QStandardItem(m_quranQuery->value(0).toString()));
            listItems.append(new QStandardItem(m_quranQuery->value(2).toString()));
            listItems.append(new QStandardItem(m_quranQuery->value(3).toString()));
            listItems.append(new QStandardItem(QString::number(m_resultStruct.scoring.at(i), 'f', 4)));
            resultModel->appendRow(listItems);
            listItems.clear();
        }
    }
    m_resultStruct.offest += maxResult;
}

void MainWindow::on_pushGoNext_clicked()
{
    QStringList headersList;
    headersList << trUtf8("سورة") << trUtf8("الآية") << trUtf8("النص") << trUtf8("التطابق");

    resultModel->clear();
    displayResults();
//    resultModel->removeRows(0, 9);
    resultModel->setHorizontalHeaderLabels(headersList);
    ui->tableView->setModel(resultModel);
    //ui->tableView->resizeColumnsToContents();
}

void MainWindow::on_tableView_doubleClicked(QModelIndex index)
{
    QDialog *dialog = new QDialog(this);
    QVBoxLayout *layout= new QVBoxLayout(dialog);
    QTextBrowser *textBrowser = new QTextBrowser(0);
    layout->addWidget(textBrowser);
    textBrowser->setText(hiText(resultModel->data(index).toString(),ui->lineQuery->text()));
    textBrowser->setAlignment(Qt::AlignRight);
    dialog->setLayout(layout);
    dialog->resize(500,500);
    dialog->show();
}

QString MainWindow::hiText(const QString &text, const QString &strToHi)
{
//    QTime time;
//    time.start();

    QStringList regExpStr = buildRegExp(strToHi);
    QString finlStr  = text;
//    QTextDocument *finalDoc  = ui->textBrowser->document();
    QTextCharFormat txtForm;

    txtForm.setBackground(Qt::yellow);
    foreach(QString regExp, regExpStr)
    {
        //        finalDoc->find(QRegExp(regExp)).setCharFormat(txtForm);
        finlStr = finlStr.replace(QRegExp(regExp), "<span style=\"background-color:yellow\">\\1</span>");
    }
//    statusBar()->showMessage(trUtf8("تم البحث في %1 ثانية").arg((double)time.elapsed() / 1000));
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

        for (int i=0; i< word.size();i++)
        {
            regExpStr.append(word.at(i));
            regExpStr.append(trUtf8("[ًٌٍَُِّْ]*"));
        }

        regExpStr.append(clPar);
        regExpList.append(regExpStr);
    }
//    qDebug() << regExpList;
    return regExpList;
}
