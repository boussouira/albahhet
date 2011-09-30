#include "quranindexerwidget.h"
#include "ui_quranindexerwidget.h"
#include "common.h"
#include "cl_common.h"
#include "bookinfo.h"
#include "indexinfo.h"
#include "bookprogresswidget.h"
#include "indexesmanager.h"
#include "arabicanalyzer.h"
#include "settingsdialog.h"
#include "settingschecker.h"
#include "quranindexer.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

QuranIndexerWidget::QuranIndexerWidget(QWidget *parent) :
    AbstractIndexingWidget(parent),
    ui(new Ui::QuranIndexerWidget)
{
    ui->setupUi(this);

    m_indexInfo = new IndexInfo();
//    m_nextButton->setText(tr("بدأ الفهرسة"));
    qDebug("OOOOOO");
}

QuranIndexerWidget::~QuranIndexerWidget()
{
    delete ui;
}

QString QuranIndexerWidget::indexTypeName()
{
    return tr("القرآن الكريم");
}

void QuranIndexerWidget::nextStep()
{
    switch(ui->stackedWidget->currentIndex()) {

    case 0:
        readPaths();
        break;

    case 1:  // Stop indexing
        stopIndexing();
        break;

    case 2:  // Done
        emit done();
        break;
    }
}

void QuranIndexerWidget::readPaths()
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

    m_bookDB->setIndexInfo(m_indexInfo);
    checkIndex();
}

void QuranIndexerWidget::checkIndex()
{
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

    startIndexing();
}

void QuranIndexerWidget::startIndexing()
{
    qDebug("Start indexing...");

//    m_cancelButton->hide();
//    m_nextButton->setText(tr("ايقاف الفهرسة"));
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->currentIndex()+1);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(114);
    ui->progressBar->setValue(0);
    ui->labelStartIndexing->setText(tr("بدأ الفهرسة على الساعة %1").arg(QDateTime::currentDateTime().toString("hh:mm")));

    QDir dir;
    QSettings settings;
    ArabicAnalyzer *analyzer = new ArabicAnalyzer();

    int ramSize = settings.value("ramSize", 100).toInt();
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

    m_indexingTime.start();

    QuranIndexer *indexThread = new QuranIndexer();
    connect(indexThread, SIGNAL(currentSoraName(QString)), SLOT(addBook(QString)));
    connect(indexThread, SIGNAL(finished()), SLOT(doneIndexing()));
    connect(indexThread, SIGNAL(indexingError()), SLOT(indexingError()));

    indexThread->setIndexInfo(m_indexInfo);
    indexThread->setBookDB(m_bookDB);
    indexThread->setWirter(m_writer);

    BookProgressWidget *progress = new BookProgressWidget(this);
    ui->widgetBooksProgress->layout()->addWidget(progress);

    connect(indexThread, SIGNAL(currentSoraName(QString)), progress, SLOT(setName(QString)));

    indexThread->start();

}

void QuranIndexerWidget::addBook(const QString &/*name*/)
{
    ui->progressBar->setValue(ui->progressBar->value()+1);
}

void QuranIndexerWidget::doneIndexing()
{
    // Indexing benchmarking
    int elpasedMsec = m_indexingTime.elapsed();

    // Optimize Index benchmarking
    int optimizeTime = -1;

    if(ui->checkOptimizeIndex->isChecked()) {
        ui->progressBar->setMaximum(114);
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

    QString msg = tr("تمت فهرسة القرآن الكريم بنجاح.");
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

    if(ui->checkShutDown->isChecked())
        shutDown();

}

void QuranIndexerWidget::indexingError()
{
    QMessageBox::warning(this,
                         tr("فهرسة المكتبة"),
                         tr("لقد حدث خطأ أثناء فهرسة المكتبة.\nالمرجوا اعادة المحاولة"));
}

void QuranIndexerWidget::stopIndexing()
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

void QuranIndexerWidget::saveIndexInfo(int indexingTime, int opimizingTime)
{
    m_indexInfo->setType(IndexInfo::QuranIndex);
    m_indexInfo->generateIndexingInfo();
    m_indexInfo->indexingInfo()->indexingTime = indexingTime;
    m_indexInfo->indexingInfo()->optimizingTime = opimizingTime;
    m_indexInfo->indexingInfo()->creatTime = QDateTime::currentDateTime().toTime_t();

    m_indexesManager->add(m_indexInfo);

    DELETE_DB(m_bookDB); // We don't need it any more, the mainwindow may open the same databases...
    m_bookDB = new BooksDB();

    emit indexCreated();
}


void QuranIndexerWidget::on_buttonSelectShamela_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 tr("اختر مجلد المكتبة الشاملة"));
    if(!path.isEmpty()) {
        ui->lineShamelaPath->setText(path);
    }
}

void QuranIndexerWidget::on_buttonSelectIndexPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
                                 tr("اختر مجلد وضع الفهرس"));
    if(!path.isEmpty()) {
        ui->lineIndexPath->setText(path);
    }
}

void QuranIndexerWidget::on_label_2_linkActivated(QString)
{
    SettingsDialog dialog(this);
    dialog.setCurrentPage(1);

    dialog.exec();
}
