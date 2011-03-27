#include "indexingdialg.h"
#include "ui_indexingdialg.h"
#include "common.h"
#include "bookinfo.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);
    hideHelpButton(this);

    m_indexInfo = new IndexInfo();
    m_bookDB = new BooksDB();

    m_stopIndexing = false;

    ui->spinThreadCount->setValue(QThread::idealThreadCount());

    setRamSize();
    ui->spinRamSize->hide();
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), SLOT(setRamSize()));
    connect(ui->pushNext, SIGNAL(clicked()), SLOT(nextStep()));
}

IndexingDialg::~IndexingDialg()
{
    delete ui;
    DELETE_DB(m_bookDB);
}

void IndexingDialg::showBooks()
{
    m_bookDB->setIndexInfo(m_indexInfo);
    m_bookDB->importBooksListFromShamela();

    m_booksCount = 0;
    m_bookDB->queryBooksToIndex();

    BookInfo *book = m_bookDB->next();
    while(book != 0) {
        ui->listWidget->insertItem(m_booksCount++, book->name());
        delete book;

        book = m_bookDB->next();
    }

    ui->label->setText(trUtf8("الكتب التي ستتم فهرستها،"
                              "\n"
                              "عدد الكتب %1:").arg(m_booksCount));
}

void IndexingDialg::startIndexing()
{
    ui->listWidget->clear();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(m_booksCount);
    ui->progressBar->setValue(0);

    m_indexedBooks = 0;

// Writer
    m_writer = NULL;
    QDir dir;
    ArabicAnalyzer *analyzer = new ArabicAnalyzer();
    if(!dir.exists(m_indexInfo->path()))
        dir.mkdir(m_indexInfo->path());
    if ( IndexReader::indexExists(qPrintable(m_indexInfo->path())) ){
        if ( IndexReader::isLocked(qPrintable(m_indexInfo->path())) ){
            IndexReader::unlock(qPrintable(m_indexInfo->path()));
        }

        m_writer = _CLNEW IndexWriter( qPrintable(m_indexInfo->path()), analyzer, true);
    }else{
        m_writer = _CLNEW IndexWriter( qPrintable(m_indexInfo->path()) ,analyzer, true);
    }
    m_writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
    m_writer->setRAMBufferSizeMB(ui->spinRamSize->value());

    m_bookDB->queryBooksToIndex();

    indexingTime.start();

    for(int i=0;i<m_threadCount;i++) {
        ShamelaIndexer *indexThread = new ShamelaIndexer();
        connect(indexThread, SIGNAL(fileIndexed(QString)), SLOT(addBook(QString)));
        connect(indexThread, SIGNAL(finished()), SLOT(doneIndexing()));
        connect(indexThread, SIGNAL(indexingError()), SLOT(indexingError()));

        indexThread->setIndexInfo(m_indexInfo);
        indexThread->setBookDB(m_bookDB);
        indexThread->setWirter(m_writer);

        indexThread->start();
    }
}

void IndexingDialg::nextStep()
{
    int i = ui->stackedWidget->currentIndex();
    if(i == 0) {
        try {
            if(ui->lineIndexName->text().isEmpty())
                throw trUtf8("لم تقم باختيار اسم الفهرس");

            if(ui->lineShamelaPath->text().isEmpty())
                throw trUtf8("لم تقم باختيار مسار المكتبة الشاملة");
            else if(!m_indexInfo->isShamelaPath(ui->lineShamelaPath->text()))
                throw trUtf8("لم تقم باختيار مسار الشاملة بشكل صحيح");

            if(ui->lineIndexPath->text().isEmpty())
                throw trUtf8("لم تقم باختيار مسار وضع الفهرس");

            QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
            QString hash = IndexInfo::indexHash(ui->lineIndexName->text());

            if(settings.childGroups().contains(hash))
                throw trUtf8("اسم الفهرس المدخل موجودا مسبقا");

        } catch(QString &e) {
            QMessageBox::warning(this, trUtf8("انشاء فهرس"), e);
            return;
        }

        m_indexInfo->setName(ui->lineIndexName->text());
        m_indexInfo->setShamelaPath(ui->lineShamelaPath->text());
        m_indexInfo->setPath(ui->lineIndexPath->text());

        checkIndex();
    } else if(i == 1) {
        m_threadCount = ui->spinThreadCount->value();
        m_indexInfo->setRamSize(ui->spinRamSize->value());
        m_indexInfo->setOptimizeIndex(ui->checkOptimizeIndex->isChecked());

        showBooks();
        ui->pushNext->setText(trUtf8("بدأ الفهرسة"));
        ui->stackedWidget->setCurrentIndex(i+1);
    } else if(i == 2) { // Start indexing
        ui->pushCancel->hide();
        ui->pushNext->setText(trUtf8("ايقاف الفهرسة"));
        ui->stackedWidget->setCurrentIndex(i+1);

        startIndexing();
    } else if(i == 3) { // Stop indexing
        stopIndexing();
    } else if(i == 4) { // Done
        saveIndexInfo();
        done(Accepted);
    }
}

void IndexingDialg::addBook(const QString &name)
{
    ui->progressBar->setValue(++m_indexedBooks);
    ui->labelIndexedBook->setText(name);

}

void IndexingDialg::doneIndexing()
{
    if(--m_threadCount <= 0) {
        // Indexing benchmarking
        int elpasedMsec = indexingTime.elapsed();

        // Optimize Index benchmarking
        int optimizeTime = -1;

        if(m_indexInfo->optimize()) {
            ui->progressBar->setMaximum(0);

            QTime optTime;
            optTime.start();
            m_writer->optimize();

            optimizeTime = optTime.elapsed();
        }

        m_writer->close();

        ui->pushNext->setText(trUtf8("انتهى"));

        ui->pushNext->setEnabled(true);
        ui->pushCancel->setEnabled(false);

        QString msg = trUtf8("تمت فهرسة %1").arg(arPlural(m_indexedBooks, BOOK, true));
        msg.append("<br>");

        msg.append(trUtf8("تمت الفهرسة خلال %1").arg(formatTime(elpasedMsec)));
        msg.append("<br>");

        if(optimizeTime != -1) {
            msg.append(trUtf8("تم ضغط الفهرس خلال %1").arg(formatTime(optimizeTime)));
            msg.append("<br>");
        }

        ui->labelIndexingInfo->setText(msg);
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);
        _CLLDELETE(m_writer);
    }
}

void IndexingDialg::indexingError()
{
    QMessageBox::warning(this,
                         trUtf8("فهرسة المكتبة"),
                         trUtf8("لقد حدث خطأ أثناء فهرسة المكتبة.\nالمرجوا اعادة المحاولة"));
}

QString IndexingDialg::formatTime(int milsec)
{
    QString time;

    int seconde = (int) ((milsec / 1000) % 60);
    int minutes = (int) (((milsec / 1000) / 60) % 60);
    int hours   = (int) (((milsec / 1000) / 60) / 60);

    if(hours > 0){
        time.append(arPlural(hours, HOUR, true));
        time.append(trUtf8(" و "));
    }

    if(minutes > 0 || hours > 0) {
        time.append(arPlural(minutes, MINUTE, true));
        time.append(trUtf8(" و "));
    }

    time.append(arPlural(seconde, SECOND, true));

    return time;
}

void IndexingDialg::stopIndexing()
{
    int rep = QMessageBox::question(this,
                                    trUtf8("فهرسة المكتبة"),
                                    trUtf8("هل تريد ايقاف فهرسة المكتبة؟"),
                                    QMessageBox::Yes|QMessageBox::No,
                                    QMessageBox::No);
    if(rep==QMessageBox::Yes){
        ui->progressBar->setMaximum(0);
        m_stopIndexing = true;
        m_bookDB->clear();
        ui->pushNext->setEnabled(false);
    }
}

void IndexingDialg::on_pushCancel_clicked()
{
    done(Rejected);
}

void IndexingDialg::setRamSize()
{
    switch(ui->comboBox->currentIndex()) {
    case 0:
        ui->spinRamSize->setValue(100);
        break;
    case 1:
        ui->spinRamSize->setValue(200);
        break;
    case 2:
        ui->spinRamSize->setValue(300);
        break;
    case 3:
        ui->spinRamSize->setValue(500);
        break;
    case 4:
        ui->spinRamSize->setValue(1000);
        break;
    case 5:
        ui->spinRamSize->setValue(1500);
        break;
    case 6:
        ui->spinRamSize->setValue(2000);
        break;
    case 7:
        ui->spinRamSize->setValue(3000);
        break;

    }
}

void IndexingDialg::on_buttonSelectShamela_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 trUtf8("اختر مجلد المكتبة الشاملة"));
    if(!path.isEmpty()) {
        ui->lineShamelaPath->setText(path);
    }
}

void IndexingDialg::on_buttonSelectIndexPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 trUtf8("اختر مجلد وضع الفهرس"));
    if(!path.isEmpty()) {
        ui->lineIndexPath->setText(path);
    }
}

void IndexingDialg::saveIndexInfo()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    QString indexName = m_indexInfo->indexHash();

    QStringList indexes = settings.value("indexes_list").toStringList();

    indexes.append(indexName);
    settings.setValue("indexes_list", indexes);

    settings.beginGroup(indexName);
    settings.setValue("name", m_indexInfo->name());
    settings.setValue("shamela_path", m_indexInfo->shamelaPath());
    settings.setValue("index_path", m_indexInfo->path());
    settings.setValue("ram_size", m_indexInfo->ramSize());
    settings.setValue("optimizeIndex", m_indexInfo->optimize());
    settings.endGroup();

    DELETE_DB(m_bookDB); // We don't need it any more, the mainwindow may open the same databases...
    m_bookDB = new BooksDB();

    emit indexCreated();
}

void IndexingDialg::checkIndex()
{
    try {
        IndexReader* r = IndexReader::open(qPrintable(m_indexInfo->path()));
//        int64_t ver = r->gtCurrentVersion(qPrintable(m_indexInfo->path()));

        int rep = QMessageBox::question(this,
                                        trUtf8("انشاء فهرس"),
                                        trUtf8("لقد تم العثور على فهرس جاهز في المسار المحدد"
                                               "<br>"
                                               "هل تريد استخدامه؟"),
                                        QMessageBox::Yes|QMessageBox::No);
        if(rep == QMessageBox::Yes) {
            ui->labelIndexingInfo->setText(trUtf8("لقد تم انشاء الفهرس بنجاح"));
            ui->pushNext->setText(trUtf8("انتهى"));
            ui->pushCancel->hide();
            ui->stackedWidget->setCurrentIndex(4);

            m_indexInfo->setRamSize(100);
        }

        r->close();
        _CLLDELETE(r);
    }
    catch(...) {}

    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);
}
