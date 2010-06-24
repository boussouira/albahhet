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

        inexQuery->exec("SELECT shamelaID, bookName, filePath FROM books ORDER BY fileSize");
        while(inexQuery->next()) {
            booksList.append(inexQuery->value(1).toString());
            m_booksCount++;
        }
        ui->listWidget->insertItems(0, booksList);
    }
    QSqlDatabase::removeDatabase("bookIndexDiaog");
}

void IndexingDialg::on_pushStartIndexing_clicked()
{
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
    m_indexing->setOptions(ui->checkOptimizeIndex->isChecked(),
                           ui->checkRamSize->isChecked() ? ui->spinRamSize->value() : 0,
                           ui->checkMaxDoc->isChecked() ? ui->spinMaxDoc->value() : 0,
                           ui->spinThreadCount->value());

    m_indexing->start();
*/
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

    indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexThread");
    indexDB.setDatabaseName("book_index.db");
    if(!indexDB.open())
        qDebug("Error opning index db");
    inexQuery = new QSqlQuery(indexDB);

    inexQuery->exec("SELECT shamelaID, bookName, filePath FROM books ORDER BY fileSize");

    if(ui->checkRamSize->isChecked())
        m_writer->setRAMBufferSizeMB(ui->spinRamSize->value());
    if(ui->checkMaxDoc->isChecked())
        m_writer->setMaxBufferedDocs(ui->spinMaxDoc->value());

    indexingTime.start();
    m_threadCount = ui->spinThreadCount->value();
    for(int i=0; i<m_threadCount;i++){
        IndexBookThread *book = new IndexBookThread(m_writer);
        connect(book, SIGNAL(giveNextBook(IndexBookThread*)), this, SLOT(nextBook(IndexBookThread*)));
        connect(book, SIGNAL(bookIsIndexed(QString)), this, SLOT(addBook(QString)));
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

void IndexingDialg::nextBook(IndexBookThread *thread)
{
    m_mutex.lock();
    if(inexQuery->next() && !m_stopIndexing) {
        thread->indexBoook(inexQuery->value(0).toString(),
                           inexQuery->value(1).toString(),
                           inexQuery->value(2).toString());
    } else {
        thread->stop();
        if(--m_threadCount <= 0)
            doneIndexing();
    }
    m_mutex.unlock();
}

void IndexingDialg::doneIndexing()
{
    if(ui->checkOptimizeIndex->isChecked())
        m_writer->optimize();

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
    delete inexQuery;
    indexDB.close();
    QSqlDatabase::removeDatabase("bookIndexThread");
    m_writer->close();
    _CLDELETE(m_writer);
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
