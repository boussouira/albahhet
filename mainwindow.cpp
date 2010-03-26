#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QStringListModel>

#define INDEX_PATH "quran_index"
#define DATABASE_PATH "quran.db"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    {
    // letters used in Quran text
    letterMap.insert("HAMZA", QChar(0x0621));
    letterMap.insert("ALEF_WITH_MADDA_ABOVE", QChar(0x0622));
    letterMap.insert("ALEF_WITH_HAMZA_ABOVE", QChar(0x0623));
    letterMap.insert("WAW_WITH_HAMZA_ABOVE", QChar(0x0624));
    letterMap.insert("ALEF_WITH_HAMZA_BELOW", QChar(0x0625));
    letterMap.insert("YEH_WITH_HAMZA", QChar(0x0626));
    letterMap.insert("ALEF", QChar(0x0627));
    letterMap.insert("BEH", QChar(0x0628));
    letterMap.insert("MARBUTA", QChar(0x0629));
    letterMap.insert("TEH", QChar(0x062A));
    letterMap.insert("THEH", QChar(0x062B));
    letterMap.insert("JEMM", QChar(0x062C));
    letterMap.insert("HAH", QChar(0x062D));
    letterMap.insert("KHAH", QChar(0x062E));
    letterMap.insert("DAL", QChar(0x062F));
    letterMap.insert("THAL", QChar(0x0630));
    letterMap.insert("REH", QChar(0x0631));
    letterMap.insert("ZAIN", QChar(0x0632));
    letterMap.insert("SEEN", QChar(0x0633));
    letterMap.insert("SHEEN", QChar(0x0634));
    letterMap.insert("SAD", QChar(0x0635));
    letterMap.insert("DAD", QChar(0x0636));
    letterMap.insert("TAH", QChar(0x0637));
    letterMap.insert("ZAH", QChar(0x0638));
    letterMap.insert("AIN", QChar(0x0639));
    letterMap.insert("GHAIN", QChar(0x063A));
    letterMap.insert("TATWEEL", QChar(0x0640));
    letterMap.insert("FEH", QChar(0x0641));
    letterMap.insert("QAF", QChar(0x0642));
    letterMap.insert("KAF", QChar(0x0643));
    letterMap.insert("LAM", QChar(0x0644));
    letterMap.insert("MEEM", QChar(0x0645));
    letterMap.insert("NOON", QChar(0x0646));
    letterMap.insert("HEH", QChar(0x0647));
    letterMap.insert("WAW", QChar(0x0648));
    letterMap.insert("ALEF_MAKSURA", QChar(0x0649));
    letterMap.insert("YEH", QChar(0x064A));
    letterMap.insert("FATHATAN", QChar(0x064B));
    letterMap.insert("DAMMATAN", QChar(0x064C));
    letterMap.insert("KASRATAN", QChar(0x064D));
    letterMap.insert("FATHA", QChar(0x064E));
    letterMap.insert("DAMMA", QChar(0x064F));
    letterMap.insert("KASRA", QChar(0x0650));
    letterMap.insert("SHADDA", QChar(0x0651));
    letterMap.insert("SUKUN", QChar(0x0652));
    letterMap.insert("MADDA", QChar(0x0653));
    letterMap.insert("HAMZA_ABOVE", QChar(0x0654));
    letterMap.insert("HAMZA_BELOW", QChar(0x0655));
    letterMap.insert("SMALL_ALEF", QChar(0x065F));
    letterMap.insert("SUPERSCRIPT_ALEF", QChar(0x0670));
    letterMap.insert("ALEF_WASLA", QChar(0x0671));
    letterMap.insert("HIGH_SALA", QChar(0x06D6));
    letterMap.insert("HIGH_GHALA", QChar(0x06D7));
    letterMap.insert("HIGH_MEEM_INITIAL_FORM", QChar(0x06D8));
    letterMap.insert("HIGH_LA", QChar(0x06D9));
    letterMap.insert("HIGH_JEMM", QChar(0x06DA));
    letterMap.insert("HIGH_THREE_DOT", QChar(0x06DB));
    letterMap.insert("HIGH_SEEN", QChar(0x06DC));
    letterMap.insert("RUB_EL_HIZB", QChar(0x06DE));
    letterMap.insert("HIGH_ROUNDED_ZERO", QChar(0x06DF));
    letterMap.insert("HIGH_UPRIGHT_ZERO", QChar(0x06E0));
    letterMap.insert("HIGH_MEEM", QChar(0x06E2));
    letterMap.insert("LOW_SEEN", QChar(0x06E3));
    letterMap.insert("SMALL_WAW", QChar(0x06E5));
    letterMap.insert("SMALL_YEH", QChar(0x06E6));
    letterMap.insert("HIGH_NOON", QChar(0x06E8));
    letterMap.insert("SAJDAH", QChar(0x06E9));
    letterMap.insert("LOW_STOP", QChar(0x06EA));
    letterMap.insert("HIGH_STOP", QChar(0x06EB));
    letterMap.insert("HIGH_STOP_FILLED", QChar(0x06EC));
    letterMap.insert("LOW_MEEM", QChar(0x06ED));
    letterMap.insert("HAMZA_ABOVE_ALEF", QChar(0x0675));
    letterMap.insert("DOTLESS_BEH", QChar(0x066E));
    letterMap.insert("HIGH_YEH", QChar(0x06E7));
    letterMap.insert("ZWNJ", QChar(0x200C));
    letterMap.insert("NBSP", QChar(0x00A0));
    letterMap.insert("NNBSP", QChar(0x202F));

    // other letters
    letterMap.insert("ARABIC_COMMA", QChar(0x060C));
    letterMap.insert("ARABIC_SEMICOLON", QChar(0x061B));

    letterMap.insert("FARSI_YEH", QChar(0x06CC));
    letterMap.insert("FARSI_HIGH_HAMZA", QChar(0x0674));
    letterMap.insert("FARSI_KEHEH", QChar(0x06A9));
    letterMap.insert("SWASH_KAF", QChar(0x06AA));
    letterMap.insert("YEH_BARREE", QChar(0x06D2));
}
//    qDebug() << letterMap;

    m_quranDB = QSqlDatabase::addDatabase("QSQLITE", "QuranTextDB");
    m_quranDB.setDatabaseName(DATABASE_PATH);

    if (!m_quranDB.open()) {
        qDebug() << "Cannot open database.";
    }
    m_quranQuery = new QSqlQuery(m_quranDB);
    m_resultModel = new QSqlQueryModel(this);

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
    IndexWriter* writer = NULL;
//    SimpleAnalyzer* an = _CLNEW SimpleAnalyzer();
    lucene::analysis::standard::StandardAnalyzer an;

    if ( IndexReader::indexExists(INDEX_PATH) ){
            if ( IndexReader::isLocked(INDEX_PATH) ){
                    printf("Index was locked... unlocking it.\n");
                    IndexReader::unlock(INDEX_PATH);
            }

            writer = _CLNEW IndexWriter( INDEX_PATH, &an, true);
    }else{
            writer = _CLNEW IndexWriter( INDEX_PATH ,&an, true);
    }
    writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);

    uint64_t str = lucene::util::Misc::currentTimeMillis();

    indexDocs(writer);
    writer->optimize();
    writer->close();
    _CLDELETE(writer);

    qDebug() << "Indexing took: " << lucene::util::Misc::currentTimeMillis() - str << " ms.";
}

void MainWindow::startSearching()
{
    try {
        //Searcher searcher(INDEX_PATH);
        standard::StandardAnalyzer analyzer;

        QString queryWord = cleanString(ui->lineQuery->text());

        if(ui->checkBox->isChecked()) {
            queryWord.replace(" ", " AND ");
        }
        const TCHAR* buf;
        IndexSearcher s(INDEX_PATH);

        // Start building the query
        Query* q = QueryParser::parse(queryWord.toStdWString().c_str(),_T("text"),&analyzer);

        buf = q->toString(_T("text"));
        qDebug() << "Searching for: " << QString::fromWCharArray(buf);
        //    _CLDELETE_CARRAY(buf);

        uint64_t str = lucene::util::Misc::currentTimeMillis();
        Hits* h = s.search(q);
        uint64_t srch = lucene::util::Misc::currentTimeMillis() - str;
        str = lucene::util::Misc::currentTimeMillis();

        QString sqlQureyString("SELECT QuranSowar.SoraName, QuranText.soraNumber, QuranText.ayaNumber, QuranText.ayaText "
                               "FROM QuranText "
                               "LEFT JOIN QuranSowar "
                               "ON QuranSowar.id = QuranText.soraNumber "
                               "WHERE QuranText.id IN (");
        int32_t i=0;
        for ( i=0;i<h->length();i++ ){
            Document* doc = &h->doc(i);

            //        qDebug() <<  i
            //                << " - Sora: " << QString::fromWCharArray(doc->get(_T("sora")))
            //                << " - Aya: "  << QString::fromWCharArray(doc->get(_T("aya")))
            //                << " - text: "  << QString::fromWCharArray(doc->get(_T("text")));
            //        */
            sqlQureyString.append(QString::fromWCharArray(doc->get(_T("id"))));
            sqlQureyString.append(",");
            //        qDebug() <<  i << " -> " << QString::number((double)h->score(i));

            //delete doc;
        }
        sqlQureyString.remove(QRegExp(",$"));
        sqlQureyString.append(")");

        m_resultModel->setQuery(sqlQureyString, m_quranDB);
        m_resultModel->setHeaderData(0, Qt::Horizontal, trUtf8("سورة"));
        m_resultModel->setHeaderData(1, Qt::Horizontal, trUtf8("رفم"));
        m_resultModel->setHeaderData(2, Qt::Horizontal, trUtf8("الاية"));
        m_resultModel->setHeaderData(3, Qt::Horizontal, trUtf8("النص"));
        ui->tableView->setWordWrap(true);
        ui->tableView->setModel(m_resultModel);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->setAlternatingRowColors(true);

        //    qDebug() << "Search took: " << srch << " ms.";
        //    qDebug() << "Screen dump took: "<< lucene::util::Misc::currentTimeMillis() - str << " ms.";
        this->statusBar()->showMessage(QString("Search took %1 ms. "
                                               "Screen dump took: %2 ms. "
                                               "Search count %3 ")
                                       .arg(srch)
                                       .arg(lucene::util::Misc::currentTimeMillis() - str)
                                       .arg(i));
        _CLDELETE(h);
        _CLDELETE(q);


        s.close();
        //delete line;
    }

    catch(CLuceneError *tmp) {
        qDebug() << "Error : " << tmp->what();
    }

    catch(...) {
        qDebug() << "Error when searching at : " << INDEX_PATH;
    }

}

void MainWindow::showStatistic()
{
    try {
        IndexReader* r = IndexReader::open(INDEX_PATH);
        qDebug() << "Statistics for " << INDEX_PATH;

        qDebug() << "Max Docs: " << r->maxDoc();
        qDebug() << "Num Docs: " << r->numDocs();

        int64_t ver = r->getCurrentVersion(INDEX_PATH);
        qDebug() << "Current Version: " << ver ;

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* empty */
        }
        qDebug() << "Term count: " << nterms;
        _CLDELETE(te);

        r->close();
        _CLDELETE(r);
    }
    catch(...) {
        qDebug() << "Error when opening : " << INDEX_PATH;
    }

}

void MainWindow::indexDocs(IndexWriter* writer)
{
    m_quranQuery->exec("SELECT QuranText.id, QuranText.ayaText, QuranText.ayaNumber, "
                       "QuranText.pageNumber, QuranText.soraNumber "
                       "FROM QuranText "
                       "ORDER BY QuranText.id ");
    while(m_quranQuery->next())
    {
        Document* doc = FileDocument(m_quranQuery->value(0).toString(),
                                     m_quranQuery->value(1).toString());
        writer->addDocument( doc );
        _CLDELETE(doc);
    }
}

Document* MainWindow::FileDocument(QString id, QString ayaText)
{
    // make a new, empty document
    Document* doc = _CLNEW Document();

    doc->add( *_CLNEW Field(_T("id"), id.toStdWString().c_str() ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
//    doc->add( *_CLNEW Field(_T("aya"), ayaNumber.toStdWString().c_str() ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
//    doc->add( *_CLNEW Field(_T("sora"), soraNumber.toStdWString().c_str() ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );
//    doc->add( *_CLNEW Field(_T("page"), pageNumber.toStdWString().c_str() ,Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );

    ayaText = cleanString(ayaText);
//    qDebug() << "CLEAN: " << ayaText;
    doc->add( *_CLNEW Field(_T("text"),ayaText.toStdWString().c_str(), Field::STORE_NO | Field::INDEX_TOKENIZED) );

    return doc;
}

QString MainWindow::cleanString(QString str)
{
//    uint64_t startTime = lucene::util::Misc::currentTimeMillis();

    str.remove(QRegExp(trUtf8("[ًٌٍَُِّْ]")));
    str.replace(letterMap["ALEF_WITH_HAMZA_ABOVE"], letterMap["ALEF"]);
    str.replace(letterMap["ALEF_WITH_HAMZA_BELOW"], letterMap["ALEF"]);
    str.replace(letterMap["ALEF_WITH_MADDA_ABOVE"], letterMap["ALEF"]);
    str.replace(letterMap["HAMZA_ABOVE_ALEF"], letterMap["ALEF"]);
    str.replace(letterMap["MARBUTA"], letterMap["HEH"]);
    str.remove(QRegExp("\\[([^\\[]+)\\]"));
    str.remove(QRegExp("\\[co\\]([^\\[]+)\\[/co\\]"));
    str.remove(QRegExp("\\(([a-zA-Z0-9]+]+)\\)"));
    str.remove(QRegExp("\\[([a-zA-Z0-9]+)\\]"));
//    uint64_t cleanTime = lucene::util::Misc::currentTimeMillis() - startTime;
//    qDebug() << QString("Clean time %1 ms.").arg(cleanTime);
    return str;
}

void MainWindow::resultsCount()
{
    this->statusBar()->showMessage(QString("Search results count: %1 ")
                                   .arg(m_resultModel->rowCount()) );
}
