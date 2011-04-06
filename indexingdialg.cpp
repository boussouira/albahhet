#include "indexingdialg.h"
#include "ui_indexingdialg.h"
#include "common.h"
#include "bookinfo.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtimeline.h>

IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);
    hideHelpButton(this);

    m_indexInfo = new IndexInfo();
    m_bookDB = new BooksDB();

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

    m_indexingTime.start();

    for(int i=0;i<m_threadCount;i++) {
        ShamelaIndexer *indexThread = new ShamelaIndexer();
        connect(indexThread, SIGNAL(currentBookName(QString)), SLOT(addBook(QString)));
        connect(indexThread, SIGNAL(finished()), SLOT(doneIndexing()));
        connect(indexThread, SIGNAL(indexingError()), SLOT(indexingError()));

        indexThread->setIndexInfo(m_indexInfo);
        indexThread->setBookDB(m_bookDB);
        indexThread->setWirter(m_writer);

        BookProgressWidget *progress = new BookProgressWidget(this);
        ui->widgetBooksProgress->layout()->addWidget(progress);

        connect(indexThread, SIGNAL(currentBookName(QString)), progress, SLOT(setName(QString)));
        connect(indexThread, SIGNAL(currentBookMax(int)), progress, SLOT(setMax(int)));
        connect(indexThread, SIGNAL(currentBookProgress(int)), progress, SLOT(setCurrentValue(int)));
        connect(progress, SIGNAL(skipCurrent()), indexThread, SLOT(skipCurrentBook()));

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

            if(m_indexesManager->nameExists(ui->lineIndexName->text()))
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
        ui->checkOptimizeIndexLast->setChecked(ui->checkOptimizeIndex->isChecked());
        ui->stackedWidget->setCurrentIndex(i+1);
    } else if(i == 2) { // Start indexing
        ui->pushCancel->hide();
        ui->pushNext->setText(trUtf8("ايقاف الفهرسة"));
        ui->stackedWidget->setCurrentIndex(i+1);

        startIndexing();
    } else if(i == 3) { // Stop indexing
        stopIndexing();
    } else if(i == 4) { // Done
        done(Accepted);
    }
}

void IndexingDialg::addBook(const QString &/*name*/)
{
    ui->progressBar->setValue(++m_indexedBooks);
}

void IndexingDialg::doneIndexing()
{
    if(--m_threadCount <= 0) {
        // Indexing benchmarking
        int elpasedMsec = m_indexingTime.elapsed();

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

        msg.append(trUtf8("تمت الفهرسة خلال %1").arg(getSizeString(elpasedMsec)));
        msg.append("<br>");

        if(optimizeTime != -1) {
            msg.append(trUtf8("تم ضغط الفهرس خلال %1").arg(getSizeString(optimizeTime)));
            msg.append("<br>");
        }

        ui->labelIndexingInfo->setText(msg);
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);
        _CLLDELETE(m_writer);

        saveIndexInfo(elpasedMsec, optimizeTime);

        if(ui->checkShutDown->isChecked())
            shutDown();
    }
}

void IndexingDialg::indexingError()
{
    QMessageBox::warning(this,
                         trUtf8("فهرسة المكتبة"),
                         trUtf8("لقد حدث خطأ أثناء فهرسة المكتبة.\nالمرجوا اعادة المحاولة"));
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
        ui->pushNext->setEnabled(false);
        m_bookDB->clear();
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

void IndexingDialg::saveIndexInfo(int indexingTime, int opimizingTime)
{
    m_indexInfo->generateIndexingInfo();
    m_indexInfo->indexingInfo()->indexingTime = indexingTime;
    m_indexInfo->indexingInfo()->optimizingTime = opimizingTime;
    m_indexInfo->indexingInfo()->creatTime = QDateTime::currentDateTime().toTime_t();

    m_indexesManager->add(m_indexInfo);

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
            saveIndexInfo();
        }

        r->close();
        _CLLDELETE(r);
    }
    catch(...) {}

    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);
}

void IndexingDialg::on_checkOptimizeIndexLast_stateChanged(int )
{
    m_indexInfo->setOptimizeIndex(ui->checkOptimizeIndexLast->isChecked());
}

void IndexingDialg::shutDown()
{
    QMessageBox msgBox(this);
    msgBox.setText(trUtf8("انتهت عملية الفهرسة بنجاح" "<br>"
                          "سيتم اطفاء الجهاز بعد %1")
                   .arg(arPlural(10,  SECOND, true)));
    msgBox.setStandardButtons(QMessageBox::Cancel);

    QTimeLine timeLine(10000, this);
    timeLine.setUpdateInterval(1000);
    timeLine.setDirection(QTimeLine::Backward);

    connect(&msgBox, SIGNAL(buttonClicked(QAbstractButton*)), &msgBox, SLOT(reject()));
    connect(&timeLine, SIGNAL(finished()), &msgBox, SLOT(accept()));
    connect(&timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(shutDownUpdateTime(qreal)));

    m_shutDownTime = 10;
    m_shutDownMsgBox = &msgBox;

    timeLine.start();
    int ret = msgBox.exec();

    if(ret) {
#ifdef Q_OS_WIN
        system("shutdown -s -t 00");
#endif
    }
}

void IndexingDialg::shutDownUpdateTime(qreal)
{
    m_shutDownMsgBox->setText(trUtf8("انتهت عملية الفهرسة بنجاح" "<br>"
                                     "سيتم اطفاء الجهاز بعد %1")
                              .arg(arPlural(m_shutDownTime--,  SECOND, true)));
}

void IndexingDialg::setIndexesManager(IndexesManager *manager)
{
    m_indexesManager = manager;
}
