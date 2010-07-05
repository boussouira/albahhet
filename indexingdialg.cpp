#include "indexthread.h"
#include "indexingdialg.h"
#include "ui_indexingdialg.h"
IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->pushStopIndexing->setVisible(false);
    ui->pushClose->setVisible(false);
    ui->spinThreadCount->setValue(QThread::idealThreadCount());
    m_stopIndexing = false;

    showBooks();
}

IndexingDialg::~IndexingDialg()
{
    delete ui;
}

void IndexingDialg::showBooks()
{
    {
        m_booksCount = 0;
        QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexDiaog");
        indexDB.setDatabaseName("book_index.db");
        if(!indexDB.open())
            qDebug("Error opning index db");
        QSqlQuery *inexQuery = new QSqlQuery(indexDB);
        QStringList booksList;

        inexQuery->exec(QString("SELECT shamelaID, bookName, filePath FROM books ORDER BY fileSize %1")
                        .arg(ui->comboBox->currentIndex() ? "DESC" : "ASC"));
        while(inexQuery->next())
            booksList.append(tr("%1 - %2").arg(++m_booksCount).arg(inexQuery->value(1).toString()));

        ui->listWidget->clear();
        ui->listWidget->insertItems(0, booksList);
//        qDebug() << "[1]:" << inexQuery->lastError().text();
    }
    QSqlDatabase::removeDatabase("bookIndexDiaog");
}

void IndexingDialg::on_pushStartIndexing_clicked()
{
    m_catsCount = 0;
    m_tempIndexs.clear();
    ui->listWidget->clear();
    ui->progressBar->setVisible(true);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(m_booksCount);
    ui->progressBar->setValue(0);
    m_indexedBooks = 0;

    ui->pushStartIndexing->setVisible(false);
    ui->groupBox->setVisible(false);
    ui->pushStopIndexing->setVisible(true);
    ui->label->setText(trUtf8("الكتب التي تمت فهرستها:"));
/*
    m_writer = NULL;
    QDir dir;
    ArabicAnalyzer *analyzer = new ArabicAnalyzer();
    if(!dir.exists(INDEX_PATH))
        dir.mkdir(INDEX_PATH);
    if ( IndexReader::indexExists(INDEX_PATH) ){
        if ( IndexReader::isLocked(INDEX_PATH) ){
            qDebug() << "Index was locked... unlocking it.";
            IndexReader::unlock(INDEX_PATH);
        }

        m_writer = _CLNEW IndexWriter( INDEX_PATH, analyzer, true);
    }else{
        m_writer = _CLNEW IndexWriter( INDEX_PATH ,analyzer, true);
    }
    m_writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
*/
    {
        QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexDiaog");
        indexDB.setDatabaseName("book_index.db");
        if(!indexDB.open())
            qDebug("Error opning index db");
        QSqlQuery *inexQuery = new QSqlQuery(indexDB);

        inexQuery->exec("SELECT MAX(cat) FROM books");
        if(inexQuery->next())
            m_catsCount = inexQuery->value(0).toInt();
    }
    QSqlDatabase::removeDatabase("bookIndexDiaog");

    indexingTime.start();
    m_threadCount = (ui->spinThreadCount->value() < m_catsCount)
                    ? ui->spinThreadCount->value() : m_catsCount;
    for(int i=0; i<m_threadCount;i++){
        IndexBookThread *book = new IndexBookThread();
        connect(book, SIGNAL(doneCatIndexing(QString)), this, SLOT(catIndexed(QString)));
        connect(book, SIGNAL(bookIsIndexed(QString)), this, SLOT(addBook(QString)));
        book->setCat(m_catsCount--);
        book->setOptions(ui->spinRamSize->value(), ui->checkOptChildIndexes->isChecked());
        book->start();
    }
}

void IndexingDialg::addBook(const QString &name)
{
    ui->progressBar->setValue(++m_indexedBooks);
    ui->listWidget->insertItem(ui->listWidget->count(), tr("%1 - %2").arg(m_indexedBooks).arg(name));
    ui->listWidget->scrollToBottom();
    if(ui->progressBar->maximum() == m_indexedBooks) {
        ui->progressBar->setMaximum(0);
        ui->pushStopIndexing->setEnabled(false);
    }
}

void IndexingDialg::catIndexed(const QString &indexFolder)
{
    m_mutex.lock();
//    qDebug() << "GOT INDEX:" << indexFolder;
    m_tempIndexs.append(indexFolder);
    if(m_catsCount > 0) {
        IndexBookThread *book = new IndexBookThread();
        connect(book, SIGNAL(doneCatIndexing(QString)), this, SLOT(catIndexed(QString)));
        connect(book, SIGNAL(bookIsIndexed(QString)), this, SLOT(addBook(QString)));
        book->setCat(m_catsCount--);
        book->setOptions(ui->spinRamSize->value(), ui->checkOptChildIndexes->isChecked());
        book->start();
    } else {
        if(--m_threadCount <= 0)
            doneIndexing();
    }
    m_mutex.unlock();
}

void IndexingDialg::doneIndexing()
{
    compineIndexs();
    int elpasedMsec = indexingTime.elapsed();
    int seconds = (int) ((elpasedMsec / 1000) % 60);
    int minutes = (int) ((elpasedMsec / 1000) / 60);

    ui->pushStopIndexing->setVisible(false);
    ui->pushClose->setVisible(true);
    ui->progressBar->setVisible(false);

    QMessageBox::information(this,
                             trUtf8("تمت الفهرسة بنجاح"),
                             trUtf8("تمت فهرسة %1 كتابا خلال <b>%2</b> و <b>%3</b>")
                             .arg(m_indexedBooks)
                             .arg(formatMinutes(minutes))
                             .arg(formatSecnds(seconds)));
    /*
    delete inexQuery;
    indexDB.close();
    QSqlDatabase::removeDatabase("bookIndexThread");
    m_writer->close();
    _CLLDELETE(m_writer);
    */
}

void IndexingDialg::compineIndexs()
{
    IndexWriter* writer = NULL;
    QDir dir;
    ArabicAnalyzer an;
    if(!dir.exists(INDEX_PATH))
        dir.mkdir(INDEX_PATH);
    if ( IndexReader::indexExists(INDEX_PATH) ){
        if ( IndexReader::isLocked(INDEX_PATH) ){
            qDebug() << "Index was locked... unlocking it.";
            IndexReader::unlock(INDEX_PATH);
        }

        writer = _CLNEW IndexWriter( INDEX_PATH, &an, true);
    }else{
        writer = _CLNEW IndexWriter( INDEX_PATH ,&an, true);
    }
    writer->setMaxFieldLength(0x7FFFFFFFL);
    writer->setUseCompoundFile(false);

    if(ui->checkRamSize->isChecked())
        writer->setRAMBufferSizeMB(ui->spinRamSize->value());

    ValueArray<Directory*> dirs(m_tempIndexs.size());
    for(int i=0; i<m_tempIndexs.size();i++){
        Directory *dir = FSDirectory::getDirectory(qPrintable(m_tempIndexs.at(i)));
        dirs[i] = dir;
    }

    writer->addIndexesNoOptimize(dirs);

    writer->setUseCompoundFile(true);
    if(ui->checkOptimizeIndex->isChecked())
        writer->optimize();

    writer->close();
    _CLLDELETE(writer);
    dirs.deleteAll();

    foreach(QString childDir, m_tempIndexs) {
        QDir indexDir(childDir);
        foreach(QString file, indexDir.entryList(QDir::NoDotAndDotDot|QDir::Files))
            indexDir.remove(file);
        indexDir.cdUp();
        indexDir.rmdir(childDir);
    }
}

void IndexingDialg::indexingError()
{
    QMessageBox::warning(this,
                         trUtf8("فهرسة المكتبة"),
                         trUtf8("لقد حدث خطأ أثناء فهرسة المكتبة.\nالمرجوا اعادة المحاولة"));
    QDir indexDir(INDEX_PATH);
    foreach(QString file, indexDir.entryList())
        indexDir.remove(file);
//    indexDir.rmdir(INDEX_PATH);
    done(1);
}

QString IndexingDialg::formatMinutes(int minutes)
{
    if(minutes == 1)
        return trUtf8("دقيقة");
    else if(minutes == 2)
        return trUtf8("دقيقتين");
    else if(9 >= minutes && minutes > 2)
        return trUtf8("%1 دقائق").arg(minutes);
    else
        return trUtf8("%1 دقيقة").arg(minutes);
}

QString IndexingDialg::formatSecnds(int seconds)
{
    if(seconds == 1)
        return trUtf8("ثانية");
    else if(seconds == 2)
        return trUtf8("ثانيتين");
    else if(9 >= seconds && seconds > 2)
        return trUtf8("%1 ثوان").arg(seconds);
    else
        return trUtf8("%1 ثانية").arg(seconds);
}

void IndexingDialg::on_pushStopIndexing_clicked()
{
    int rep = QMessageBox::question(this,
                                    trUtf8("فهرسة المكتبة"),
                                    trUtf8("هل تريد ايقاف فهرسة المكتبة"),
                                    QMessageBox::Yes|QMessageBox::No);
    if(rep==QMessageBox::Yes){
        m_stopIndexing = true;
        ui->pushStopIndexing->setEnabled(false);
        ui->progressBar->setMaximum(0);
    }
}

void IndexingDialg::on_pushClose_clicked()
{
    done(0);
}

void IndexingDialg::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    showBooks();
}
