#include "shamelaupdaterdialog.h"
#include "ui_shamelaupdaterdialog.h"
#include "common.h"
#include "cl_common.h"
#include "shamelaindexinfo.h"
#include "booksdb.h"
#include "shamelaselectbookdialog.h"
#include "arabicanalyzer.h"
#include "shamelaindexer.h"

#include <exception>
#include <iostream>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qdir.h>
#include <qdebug.h>

ShamelaUpdaterDialog::ShamelaUpdaterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShamelaUpdaterDialog)
{
    ui->setupUi(this);
}

ShamelaUpdaterDialog::~ShamelaUpdaterDialog()
{
    delete ui;
}

void ShamelaUpdaterDialog::setBooksDB(BooksDB *db)
{
    m_bookDb = db;
    m_updater.setIndexInfo(db->indexInfo());

    getUpdateBooks();
}

void ShamelaUpdaterDialog::on_pushNext_clicked()
{
    switch(ui->stackedWidget->currentIndex()) {
    case 0:
        startUpdate();
        ui->stackedWidget->setCurrentIndex(1);
        ui->pushNext->setText(tr("انتهى"));
        ui->pushCancel->hide();
        break;

    case 1:
        accept();
        break;
    }
}

void ShamelaUpdaterDialog::on_pushCancel_clicked()
{
    reject();
}

void ShamelaUpdaterDialog::getUpdateBooks()
{
    m_updater.loadBooks();
    QList<QStandardItem *> items = m_updater.getTaskItems();

    m_model = new QStandardItemModel(ui->listView);

    foreach (QStandardItem *item, items) {
        m_model->appendRow(item);
    }

    ui->listView->setModel(m_model);
}

void ShamelaUpdaterDialog::startUpdate()
{
    int progressMax = 5;

    // Progress dialog
    QProgressDialog progress(tr("جاري تحديث فهرس %1...").arg(m_bookDb->indexInfo()->name()),
                             QString(),
                             0,
                             progressMax,
                             this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setWindowTitle(windowTitle());
    progress.show();

    QList<int> booksToAdd = m_updater.getBooksToAdd();
    QList<int> booksToDelete = m_updater.getBooksToDelete();

    if(booksToDelete.count() > 0) {

        PROGRESS_DIALOG_STEP("حذف الكتب من قاعدة البيانات");
        QStringList deletedBooksName = m_bookDb->removeBooks(booksToDelete);

        PROGRESS_DIALOG_STEP("حذف الكتب من الفهرس");
        deletBooksFromIndex(booksToDelete, m_bookDb->indexInfo());

        ui->labelDeletedBooks->setText(tr("تم حذف %1:")
                                       .arg(arPlural(deletedBooksName.count(), BOOK, true)));
        ui->listDeletedBooks->addItems(deletedBooksName);
        ui->widgetDeletedBooks->show();
    } else {
        ui->widgetDeletedBooks->hide();
    }

    if(booksToAdd.count() > 0) {

        PROGRESS_DIALOG_STEP("اضافة الكتب الجديدة الى قاعدة البيانات");
        QStringList addedBooksName = m_bookDb->addBooks(booksToAdd);

        PROGRESS_DIALOG_STEP("فهرسة الكتب الجديدة");
        indexBooks(booksToAdd, m_bookDb, m_bookDb->indexInfo());

        ui->labelAddedBooks->setText(tr("تم اضافة %1:")
                                     .arg(arPlural(addedBooksName.count(), BOOK, true)));
        ui->listAddedBooks->addItems(addedBooksName);

        ui->widgetAddedBooks->show();
    } else {
        ui->widgetAddedBooks->hide();
    }

    if(booksToAdd.count() > 0 || booksToDelete.count() > 0) {
        ui->labelNoChange->hide();
        ui->splitter->show();
    } else {
        ui->labelNoChange->show();
        ui->splitter->hide();
    }

    progress.setValue(progressMax);
}

void ShamelaUpdaterDialog::indexBooks(QList<int> ids, BooksDB *bookDB, ShamelaIndexInfo *info)
{
    try {
        IndexWriter *writer;
        QSettings settings;
        int ramSize = settings.value("ramSize", 100).toInt();

        QDir dir;
        ArabicAnalyzer *analyzer = new ArabicAnalyzer();
        if(!dir.exists(info->indexPath()))
            dir.mkdir(info->indexPath());
        if ( IndexReader::indexExists(qPrintable(info->indexPath())) ){
            if ( IndexReader::isLocked(qPrintable(info->indexPath())) ){
                IndexReader::unlock(qPrintable(info->indexPath()));
            }

            writer = _CLNEW IndexWriter( qPrintable(info->indexPath()), analyzer, false);
        } else {
            QMessageBox::critical(this,
                                  tr("خطأ عند التحديث"),
                                  tr("لم يتم العثور على اي فهرس في المسار" "\n" "%1").arg(info->path()));
            return;
        }

        writer->setUseCompoundFile(false);
        writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
        writer->setRAMBufferSizeMB(ramSize);

        bookDB->queryBooksToIndex(ids);

        ShamelaIndexer *indexThread = new ShamelaIndexer();
        indexThread->setIndexInfo(info);
        indexThread->setBookDB(bookDB);
        indexThread->setWirter(writer);

        indexThread->run();

        writer->close();
        _CLDELETE(writer);
        _CLDELETE(indexThread);
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "CLucene Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
    }
    catch(std::exception &err){
        QMessageBox::warning(0, "Error when Indexing",
                             tr("exception: %1").arg(err.what()));
    }
    catch(...){
        QMessageBox::warning(0, "Unkonw error when Indexing",
                             tr("Unknow error"));
    }
}

void ShamelaUpdaterDialog::deletBooksFromIndex(QList<int> ids, ShamelaIndexInfo *info)
{

    try {
        IndexWriter *writer;
        ArabicAnalyzer *analyzer = new ArabicAnalyzer();

        QSettings settings;
        int ramSize = settings.value("ramSize", 100).toInt();

        QDir dir;
        if(!dir.exists(info->indexPath()))
            dir.mkdir(info->indexPath());
        if ( IndexReader::indexExists(qPrintable(info->indexPath())) ){
            if ( IndexReader::isLocked(qPrintable(info->indexPath())) ){
                IndexReader::unlock(qPrintable(info->indexPath()));
            }

            writer = _CLNEW IndexWriter( qPrintable(info->indexPath()), analyzer, false);
        } else {
            QMessageBox::critical(this,
                                  tr("خطأ عند التحديث"),
                                  tr("لم يتم العثور على اي فهرس في المسار" "\n" "%1").arg(info->path()));
            return;
        }

        writer->setUseCompoundFile(false);
        writer->setRAMBufferSizeMB(ramSize);

        for(int i=0; i<ids.count(); i++) {
            Term *term = new Term(BOOK_ID_FIELD, QStringToTChar(QString::number(ids.at(i))));
            writer->deleteDocuments(term);
//            _CLDECDELETE(term);
        }

        writer->close();
        _CLDELETE(writer);
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "CLucene Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
    }
    catch(std::exception &err){
        QMessageBox::warning(0, "Error when Indexing",
                             tr("exception: %1").arg(err.what()));
    }
    catch(...){
        QMessageBox::warning(0, "Unkonw error when Indexing",
                             tr("Unknow error"));
    }
}

void ShamelaUpdaterDialog::on_toolAdd_clicked()
{
    ShamelaSelectBookDialog selectDialog(m_bookDb, this);
    if(selectDialog.exec()) {
        ShamelaUpdaterTask task;
        task.bookID = selectDialog.selectedBookID;
        task.bookVersion= selectDialog.selectedBookVersion;
        task.bookName = selectDialog.selectedBookName;
        task.task = ShamelaUpdaterTask::Update;

        m_updater.addTask(task);
        m_addedTasks.append(task);

        QList<QStandardItem *> items = m_updater.getTaskItems();

        m_model->clear();
        foreach (QStandardItem *item, items) {
            m_model->appendRow(item);
        }
    }
}

void ShamelaUpdaterDialog::on_toolDelete_clicked()
{
    if(ui->listView->selectionModel()->selectedIndexes().isEmpty()) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("لم تقم باختيار اي كتاب"));
        return;
    }

    QModelIndex index = ui->listView->selectionModel()->selectedIndexes().first();

    ShamelaUpdaterTask task;
    task.fromString(index.data(ShamelaUpdater::taskStringRole).toString());

    if(!m_addedTasks.contains(task)) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("لا يمكنك حذف الكتب التي تم التعرف عليها بشكل تلقائي"));
        return;
    }

    if(m_updater.removeTask(task)) {
        m_model->clear();

        QList<QStandardItem *> items = m_updater.getTaskItems();

        foreach (QStandardItem *item, items) {
            m_model->appendRow(item);
        }
    }

    m_addedTasks.removeOne(task);
}
