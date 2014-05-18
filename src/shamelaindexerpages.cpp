#include "shamelaindexerpages.h"
#include "shamelaindexerwidget.h"
#include "common.h"
#include "cl_common.h"
#include "bookinfo.h"
#include "shamelaindexinfo.h"
#include "bookprogresswidget.h"
#include "indexesmanager.h"
#include "shamelaindexer.h"
#include "settingsdialog.h"
#include "settingschecker.h"
#include "filechooserwidget.h"
#include "indexoptimizer.h"
#include "booksdb.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qprogressbar.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>


/****** ShamelaPageBase ********/
ShamelaPageBase::ShamelaPageBase(ShamelaIndexerWidget *parent) :
    QWizardPage(parent), m_parent(parent) {}

bool ShamelaPageBase::cancel()
{
    return true;
}

/****** ShamelaPageBase ********/

/****** SelectShamelaPage ********/
SelectShamelaPage::SelectShamelaPage(ShamelaIndexerWidget *parent) : ShamelaPageBase(parent)
{
    setTitle(tr("المكتبة الشاملة"));
    setSubTitle(tr("قم باختيار مجلد المكتبة الشاملة ومجلد وضع الفهرس"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Index name line edit
    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel(tr("اسم الفهرس:"));
    hbox->addWidget(label);

    QLineEdit *lineIndexName = new QLineEdit(this);
    hbox->addWidget(lineIndexName);

    hbox->addStretch();
    layout->addLayout(hbox);

    // Shamela path chooser
    FileChooserWidget *shamelaChooser = new FileChooserWidget(this);
    shamelaChooser->setLabelText(tr("المكتبة الشاملة:"));
    layout->addWidget(shamelaChooser);

    // Index directory path
    FileChooserWidget *indexChooser = new FileChooserWidget(this);
    indexChooser->setLabelText(tr("مجلد وضع الفهرس:"));
    layout->addWidget(indexChooser);

    layout->addStretch();

    QLabel *labelSettings = new QLabel(this);
    labelSettings->setText(QString("<a href=\"#conf\">%1</a>").arg(tr("اعدادات الفهرسة")));
    connect(labelSettings, SIGNAL(linkActivated(QString)), SLOT(openSettings()));

    layout->addWidget(labelSettings);

    // TODO: Remove this hack
    label->setMinimumWidth(indexChooser->label()->width());
    shamelaChooser->label()->setMinimumWidth(indexChooser->label()->width());
    indexChooser->label()->setMinimumWidth(indexChooser->label()->width());

    registerField("index.name*", lineIndexName);
    registerField("index.shamela.path*", shamelaChooser->lineEdit());
    registerField("index.outdir.path*", indexChooser->lineEdit());
}

void SelectShamelaPage::openSettings()
{
    SettingsDialog dialog(this);
     dialog.setCurrentPage(1);

     dialog.exec();
}

bool SelectShamelaPage::validatePage()
{
    try {
        QString indexName = field("index.name").toString().trimmed();
        if(indexName.isEmpty())
            throw tr("لم تقم باختيار اسم الفهرس");

        if(m_parent->indexesManager()->nameExists(indexName))
            throw tr("اسم الفهرس المدخل موجودا مسبقا");

        QString shamelaPath = field("index.shamela.path").toString().trimmed();
        if(shamelaPath.isEmpty())
            throw tr("لم تقم باختيار مسار المكتبة الشاملة");
        else if(!m_parent->indexInfo()->isShamelaPath(shamelaPath))
            throw tr("لم تقم باختيار مسار الشاملة بشكل صحيح");

        QString outdirPath = field("index.outdir.path").toString().trimmed();
        if(outdirPath.isEmpty())
            throw tr("لم تقم باختيار مسار وضع الفهرس");

        m_parent->indexInfo()->setName(indexName);
        m_parent->indexInfo()->setShamelaPath(shamelaPath);
        m_parent->indexInfo()->setPath(outdirPath);

        return true;
    } catch(QString &e) {
        QMessageBox::warning(this, title(), e);
        return false;
    }
}
/****** SelectShamelaPage ********/

/****** SelectBooksPage ********/
SelectBooksPage::SelectBooksPage(ShamelaIndexerWidget *parent) : ShamelaPageBase(parent)
{
    setTitle(tr("المكتبة الشاملة"));
    setSubTitle(tr("الكتب التي سيتم فهرستها"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_listWidget = new QListWidget(this);
    layout->addWidget(m_listWidget);

    m_useExistingIndex = false;
}

void SelectBooksPage::initializePage()
{
    checkIndex();
}

bool SelectBooksPage::validatePage()
{
    return true;
}

int SelectBooksPage::nextId() const
{
    if(m_useExistingIndex)
        return ShamelaIndexerWidget::Page_DoneIndexingPage;
    else
        return QWizardPage::nextId();
}

void SelectBooksPage::checkIndex()
{
    QDir dir(m_parent->indexInfo()->indexPath());
    if(!dir.exists()) {
        qDebug("Create index path...");
        dir.mkpath(m_parent->indexInfo()->indexPath());
    }

    try {
        IndexReader* r = IndexReader::open(qPrintable(m_parent->indexInfo()->indexPath()));
        //        int64_t ver = r->gtCurrentVersion(qPrintable(m_wizard->indexInfo()->path()));

        r->close();
        _CLLDELETE(r);

        int rep = QMessageBox::question(this,
                                        tr("انشاء فهرس"),
                                        tr("لقد تم العثور على فهرس جاهز في المسار المحدد"
                                           "<br>"
                                           "هل تريد استخدامه؟"),
                                        QMessageBox::Yes|QMessageBox::No);
        if(rep == QMessageBox::Yes) {
            //                ui->labelIndexingInfo->setText(tr("لقد تم انشاء الفهرس بنجاح"));
            //                m_nextButton->setText(tr("انتهى"));
            //                m_cancelButton->hide();
            //                ui->stackedWidget->setCurrentIndex(3);

            m_parent->saveIndexInfo();
            m_useExistingIndex = true;
            wizard()->next();
            return;
        }
    }
    catch(...) {}

    showBooks();
}

void SelectBooksPage::showBooks()
{
    m_parent->bookDB()->setIndexInfo(m_parent->indexInfo());
    m_parent->bookDB()->importBooksListFromShamela();

    m_listWidget->clear();

    int booksCount = 0;
    m_parent->bookDB()->queryBooksToIndex();

    BookInfo *book = m_parent->bookDB()->next();
    while(book) {
        m_listWidget->addItem(book->name());
        delete book;

        ++booksCount;
        book = m_parent->bookDB()->next();
    }

    m_parent->setBooksCount(booksCount);
}
/****** SelectBooksPage ********/

/****** BooksIndexingPage ********/
BooksIndexingPage::BooksIndexingPage(ShamelaIndexerWidget *parent) : ShamelaPageBase(parent)
{
    setTitle(tr("فهرسة الكتب"));
    setSubTitle(tr("من فضلك انتظر حتى تتم فهرسة الكتب"));

    m_optimizer = 0;
    m_done = false;

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_bookNameBox = new QVBoxLayout();
    layout->addLayout(m_bookNameBox);

    layout->addStretch();

    QGroupBox *groupBox = new QGroupBox(this);
    groupBox->setTitle(tr("تقدم الفهرسة"));
    groupBox->setLayout(new QVBoxLayout());

    m_progressBar = new QProgressBar(groupBox);
    m_progressBar->setAlignment(Qt::AlignCenter);
    groupBox->layout()->addWidget(m_progressBar);
    layout->addWidget(groupBox);

    m_checkOptimizeIndex = new QCheckBox(this);
    m_checkOptimizeIndex->setText(tr("ضغط الفهرس عند انتهاء الفهرسة"));
    layout->addWidget(m_checkOptimizeIndex);

    /*
    QCheckBox *checkShutDown = new QCheckBox(this);
    checkShutDown->setText(tr("اطفاء الجهاز عند الانتهاء"));
    layout->addWidget(checkShutDown);
    */
}

void BooksIndexingPage::initializePage()
{
    startIndexing();
}

bool BooksIndexingPage::isComplete() const
{
    return m_done;
}

bool BooksIndexingPage::cancel()
{
    int rep = QMessageBox::question(this,
                                    tr("فهرسة المكتبة"),
                                    tr("هل تريد ايقاف فهرسة المكتبة؟"),
                                    QMessageBox::Yes|QMessageBox::No,
                                    QMessageBox::No);
    if(rep==QMessageBox::Yes){
        m_progressBar->setMaximum(0);
        m_parent->setApplicationProgressVisible(true, true);
        m_parent->bookDB()->clear();
    }

    return false;
}

void BooksIndexingPage::startIndexing()
{
    qDebug("Start indexing...");

    m_parent->enableHibernateMode(false);

    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(m_parent->booksCount());
    m_progressBar->setValue(0);

    m_parent->initInternal();
    m_parent->setApplicationProgressVisible(true);
    m_parent->setApplicationProgressRange(0, m_parent->booksCount());
    m_parent->setApplicationProgressValue(0);

    //ui->labelStartIndexing->setText(tr("بدأ الفهرسة على الساعة %1").arg(QDateTime::currentDateTime().toString("hh:mm")));

    m_indexedBooks = 0;

    QDir dir;
    QSettings settings;

    m_threadCount = settings.value("threadCount", QThread::idealThreadCount()).toInt();
    m_checkOptimizeIndex->setChecked(settings.value("optimizeIndex", false).toBool());
    bool showBookIndexProgress = true;

    if(!dir.exists(m_parent->indexInfo()->indexPath()))
        dir.mkdir(m_parent->indexInfo()->indexPath());
    if(IndexReader::indexExists(qPrintable(m_parent->indexInfo()->indexPath()))) {
        if(IndexReader::isLocked(qPrintable(m_parent->indexInfo()->indexPath()))) {
            IndexReader::unlock(qPrintable(m_parent->indexInfo()->indexPath()));
        }
    }

    m_parent->bookDB()->queryBooksToIndex();
    m_parent->openIndexWriter(m_parent->indexInfo()->indexPath());

    m_indexingTime.start();

    for(int i=0;i<m_threadCount;i++) {
        ShamelaIndexer *indexThread = new ShamelaIndexer();
        connect(indexThread, SIGNAL(currentBookName(QString)), SLOT(addBook(QString)));
        connect(indexThread, SIGNAL(finished()), SLOT(indexThreadFinished()));
        connect(indexThread, SIGNAL(indexingError()), SLOT(indexThreadError()));

        indexThread->setIndexInfo(m_parent->indexInfo());
        indexThread->setBookDB(m_parent->bookDB());
        indexThread->setWirter(m_parent->indexWriter());

        BookProgressWidget *progress = new BookProgressWidget(true, this);
        m_bookProgressList.append(progress);
        m_bookNameBox->addWidget(progress);

        connect(indexThread, SIGNAL(currentBookName(QString)), progress, SLOT(setName(QString)));
        connect(indexThread, SIGNAL(finished()), progress, SLOT(hide()));

        if(showBookIndexProgress) {
            connect(indexThread, SIGNAL(currentBookMax(int)), progress, SLOT(setTotalProgress(int)));
            connect(indexThread, SIGNAL(currentBookProgress(int)), progress, SLOT(setProgress(int)));
        }

        indexThread->start();
    }
}

void BooksIndexingPage::addBook(const QString &)
{
    m_progressBar->setValue(++m_indexedBooks);
    m_parent->setApplicationProgressValue(m_indexedBooks);
}

void BooksIndexingPage::indexThreadFinished()
{
    if(--m_threadCount > 0) {
        // One or more thread is still indexing
        return;
    }

    m_progressBar->setMaximum(m_indexedBooks);
    m_progressBar->setValue(m_progressBar->maximum());

    m_parent->setIndexedBooksCount(m_indexedBooks);
    m_parent->setIndexingTime(m_indexingTime.elapsed());

    wizard()->button(QWizard::CancelButton)->setEnabled(false);

    // Show infinity progress bar
    m_progressBar->setMaximum(0);
    m_parent->setApplicationProgressVisible(true, true);

    m_optimizer = new IndexOptimizer(this);
    m_optimizer->setIndexWriter(m_parent->indexWriter());
    m_optimizer->setOptimizeIndex(m_checkOptimizeIndex->isChecked());
    m_optimizer->setCloseIndex(true);

    connect(m_optimizer, SIGNAL(finished()), SLOT(indexingDone()));

    if(m_checkOptimizeIndex->isChecked()) {
        setSubTitle(tr("من فضلك انتظر حتى يتم ضغط الفهرس"));

        QGroupBox *groupBox = qobject_cast<QGroupBox*>(m_progressBar->parent());
        if(groupBox)
            groupBox->setTitle(tr("ضغط الفهرس"));
    }

    m_checkOptimizeIndex->setEnabled(false);

    m_optimizer->start();
}

void BooksIndexingPage::indexThreadError()
{
    QMessageBox::warning(this,
                         tr("فهرسة المكتبة"),
                         tr("لقد حدث خطأ أثناء فهرسة المكتبة.\nالمرجوا اعادة المحاولة"));
}

void BooksIndexingPage::indexingDone()
{
    if(m_optimizer && m_optimizer->optimizeIndex())
        m_parent->setOptimizeIndexTime(m_optimizer->optimizeTime());

    // The index optimizer thread will close and delete the index writer
    m_parent->setIndexWriter(0);

    m_parent->saveIndexInfo();

    m_parent->enableHibernateMode(true);

    wizard()->setOption(QWizard::NoCancelButton);

    m_parent->setApplicationProgressVisible(false);
    m_parent->cleanup();

    m_done = true;
    emit completeChanged();

//        if(ui->checkShutDown->isChecked())
//            shutDown();

    QTimer::singleShot(100, wizard(), SLOT(next()));
}

/****** BooksIndexingPage ********/


DoneIndexingPage::DoneIndexingPage(ShamelaIndexerWidget *parent) : ShamelaPageBase(parent)
{
    setTitle(tr("انتهاء الفهرسة"));
    setSubTitle(tr("لقد تم الانتهاء من فهرسة الكتب"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_label = new QLabel(this);
    layout->addWidget(m_label);

    layout->addStretch();
}

void DoneIndexingPage::initializePage()
{
    QString info;

    if(m_parent->indexingTime() == -1 && m_parent->optimizeIndexTime() == -1) {
        info = tr("لقد تمت اضافة الفهرس الى البرنامج بنجاح");
    } else {
        info = tr("تمت فهرسة %1").arg(arPlural(m_parent->indexedBooksCount(), BOOK, true));
        info.append("<br>");

        info.append(tr("تمت الفهرسة خلال %1").arg(getTimeString(m_parent->indexingTime())));
        info.append("<br>");

        if(m_parent->optimizeIndexTime() != -1) {
            info.append(tr("تم ضغط الفهرس خلال %1").arg(getTimeString(m_parent->optimizeIndexTime())));
            info.append("<br>");
        }
    }

    m_label->setText(info);
}
