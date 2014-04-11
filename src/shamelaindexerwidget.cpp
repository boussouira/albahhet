#include "shamelaindexerwidget.h"
#include "ui_shamelaindexerwidget.h"
#include "common.h"
#include "cl_common.h"
#include "bookinfo.h"
#include "indexinfo.h"
#include "bookprogresswidget.h"
#include "indexesmanager.h"
#include "arabicanalyzer.h"
#include "shamelaindexer.h"
#include "settingsdialog.h"
#include "settingschecker.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <Windows.h>

ShamelaIndexerWidget::ShamelaIndexerWidget(QWidget *parent) :
    AbstractIndexingWidget(parent),
    ui(new Ui::ShamelaIndexerWidget)
{
    ui->setupUi(this);

    m_indexInfo = new IndexInfo();
}

ShamelaIndexerWidget::~ShamelaIndexerWidget()
{
    delete ui;
    DELETE_DB(m_bookDB);
}

void ShamelaIndexerWidget::nextStep()
{
    switch(ui->stackedWidget->currentIndex()) {

    case 0:
        readPaths();
        break;

    case 1:  // Start indexing
        startIndexing();
        break;

    case 2:  // Stop indexing
        stopIndexing();
        break;

    case 3:  // Done
        emit done();
        break;
    }
}

void ShamelaIndexerWidget::readPaths()
{
    try {
        if(ui->lineIndexName->text().isEmpty())
            throw tr("لم تقم باختيار اسم الفهرس");

        if(ui->lineShamelaPath->text().isEmpty())
            throw tr("لم تقم باختيار مسار المكتبة الشاملة");
        else if(!m_indexInfo->isShamelaPath(ui->lineShamelaPath->text()))
            throw tr("لم تقم باختيار مسار الشاملة بشكل صحيح");

        if(ui->lineIndexPath->text().isEmpty())
            throw tr("لم تقم باختيار مسار وضع الفهرس");

        if(m_indexesManager->nameExists(ui->lineIndexName->text()))
            throw tr("اسم الفهرس المدخل موجودا مسبقا");

    } catch(QString &e) {
        QMessageBox::warning(this, tr("انشاء فهرس"), e);
        return;
    }

    m_indexInfo->setName(ui->lineIndexName->text());
    m_indexInfo->setShamelaPath(ui->lineShamelaPath->text());
    m_indexInfo->setPath(ui->lineIndexPath->text());
    checkIndex();
}

void ShamelaIndexerWidget::checkIndex()
{
    QDir dir(m_indexInfo->indexPath());
    if(!dir.exists()) {
        qDebug("Create index path...");
        dir.mkpath(m_indexInfo->indexPath());
    }

    try {
        IndexReader* r = IndexReader::open(qPrintable(m_indexInfo->indexPath()));
//        int64_t ver = r->gtCurrentVersion(qPrintable(m_indexInfo->path()));

        int rep = QMessageBox::question(this,
                                        tr("انشاء فهرس"),
                                        tr("لقد تم العثور على فهرس جاهز في المسار المحدد"
                                               "<br>"
                                               "هل تريد استخدامه؟"),
                                        QMessageBox::Yes|QMessageBox::No);
        if(rep == QMessageBox::Yes) {
            ui->labelIndexingInfo->setText(tr("لقد تم انشاء الفهرس بنجاح"));
            m_nextButton->setText(tr("انتهى"));
            m_cancelButton->hide();
            ui->stackedWidget->setCurrentIndex(3);

            saveIndexInfo();
            return;
        }

        r->close();
        _CLLDELETE(r);

    }
    catch(...) {}

    showBooks();
    m_nextButton->setText(tr("بدأ الفهرسة"));
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);
}

void ShamelaIndexerWidget::showBooks()
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

    ui->label->setText(tr("الكتب التي ستتم فهرستها،"
                              "\n"
                              "عدد الكتب %1:").arg(m_booksCount));
}

void ShamelaIndexerWidget::startIndexing()
{
    qDebug("Start indexing...");

    if(SetThreadExecutionState(ES_SYSTEM_REQUIRED|ES_CONTINUOUS) == NULL) {
        qWarning("Error when preventing system from going to hibernate mode");
    }

    m_cancelButton->hide();
    m_nextButton->setText(tr("ايقاف الفهرسة"));
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);

    ui->listWidget->clear();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(m_booksCount);
    ui->progressBar->setValue(0);
    ui->labelStartIndexing->setText(tr("بدأ الفهرسة على الساعة %1").arg(QDateTime::currentDateTime().toString("hh:mm")));

    m_indexedBooks = 0;

    QDir dir;
    QSettings settings;
    ArabicAnalyzer *analyzer = new ArabicAnalyzer();

    int ramSize = settings.value("ramSize", 100).toInt();
    m_threadCount = settings.value("threadCount", QThread::idealThreadCount()).toInt();
    ui->checkOptimizeIndex->setChecked(settings.value("optimizeIndex", false).toBool());

    if(!dir.exists(m_indexInfo->indexPath()))
        dir.mkdir(m_indexInfo->indexPath());
    if(IndexReader::indexExists(qPrintable(m_indexInfo->indexPath()))) {
        if(IndexReader::isLocked(qPrintable(m_indexInfo->indexPath()))) {
            IndexReader::unlock(qPrintable(m_indexInfo->indexPath()));
        }
    }

    m_writer = _CLNEW IndexWriter( qPrintable(m_indexInfo->indexPath()) ,analyzer, true);

    m_writer->setUseCompoundFile(false);
    m_writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
    m_writer->setRAMBufferSizeMB(ramSize);
    m_writer->setMergeFactor(25);

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

        indexThread->start();
    }
}

void ShamelaIndexerWidget::addBook(const QString &/*name*/)
{
    ui->progressBar->setValue(++m_indexedBooks);
}

void ShamelaIndexerWidget::doneIndexing()
{
    if(--m_threadCount <= 0) {
        // Indexing benchmarking
        int elpasedMsec = m_indexingTime.elapsed();

        // Optimize Index benchmarking
        int optimizeTime = -1;

        if(ui->checkOptimizeIndex->isChecked()) {
            ui->progressBar->setMaximum(m_indexedBooks);
            ui->progressBar->setValue(ui->progressBar->maximum());
            ui->labelStartIndexing->setText(tr("جاري ضغط الفهرس"));
            ui->widgetBooksProgress->hide();
            ui->checkOptimizeIndex->setEnabled(false);

            qApp->processEvents();

            QTime optTime;
            optTime.start();
            m_writer->optimize(MAX_SEGMENT);

            optimizeTime = optTime.elapsed();
        }

        m_writer->close();

        m_nextButton->setText(tr("انتهى"));

        m_nextButton->setEnabled(true);
        m_cancelButton->setEnabled(false);

        QString msg = tr("تمت فهرسة %1").arg(arPlural(m_indexedBooks, BOOK, true));
        msg.append("<br>");

        msg.append(tr("تمت الفهرسة خلال %1").arg(getTimeString(elpasedMsec)));
        msg.append("<br>");

        if(optimizeTime != -1) {
            msg.append(tr("تم ضغط الفهرس خلال %1").arg(getTimeString(optimizeTime)));
            msg.append("<br>");
        }

        ui->labelIndexingInfo->setText(msg);
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);
        _CLLDELETE(m_writer);

        saveIndexInfo(elpasedMsec, optimizeTime);

        SetThreadExecutionState(ES_CONTINUOUS);

        if(ui->checkShutDown->isChecked())
            shutDown();
    }
}

void ShamelaIndexerWidget::indexingError()
{
    QMessageBox::warning(this,
                         tr("فهرسة المكتبة"),
                         tr("لقد حدث خطأ أثناء فهرسة المكتبة.\nالمرجوا اعادة المحاولة"));
}

void ShamelaIndexerWidget::stopIndexing()
{
    int rep = QMessageBox::question(this,
                                    tr("فهرسة المكتبة"),
                                    tr("هل تريد ايقاف فهرسة المكتبة؟"),
                                    QMessageBox::Yes|QMessageBox::No,
                                    QMessageBox::No);
    if(rep==QMessageBox::Yes){
        ui->progressBar->setMaximum(0);
        m_nextButton->setEnabled(false);
        m_bookDB->clear();
    }
}

void ShamelaIndexerWidget::saveIndexInfo(int indexingTime, int opimizingTime)
{
    m_indexInfo->setType(IndexInfo::ShamelaIndex);
    m_indexInfo->generateIndexingInfo();
    m_indexInfo->indexingInfo()->indexingTime = indexingTime;
    m_indexInfo->indexingInfo()->optimizingTime = opimizingTime;
    m_indexInfo->indexingInfo()->creatTime = QDateTime::currentDateTime().toTime_t();

    m_indexesManager->add(m_indexInfo);

    DELETE_DB(m_bookDB); // We don't need it any more, the mainwindow may open the same databases...
    m_bookDB = new BooksDB();

    emit indexCreated();
}

void ShamelaIndexerWidget::on_buttonSelectShamela_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 tr("اختر مجلد المكتبة الشاملة"));
    if(!path.isEmpty()) {
        ui->lineShamelaPath->setText(path);
    }
}

void ShamelaIndexerWidget::on_buttonSelectIndexPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 tr("اختر مجلد وضع الفهرس"));
    if(!path.isEmpty()) {
        ui->lineIndexPath->setText(path);
    }
}

void ShamelaIndexerWidget::on_label_2_linkActivated(QString)
{
    SettingsDialog dialog(this);
    dialog.setCurrentPage(1);

    dialog.exec();
}

QString ShamelaIndexerWidget::indexTypeName()
{
    return tr("المكتبة الشاملة");
}
