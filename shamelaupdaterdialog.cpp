#include "shamelaupdaterdialog.h"
#include "ui_shamelaupdaterdialog.h"
#include "common.h"
#include "cl_common.h"
#include "indexinfo.h"
#include "booksdb.h"
#include "indexingdialg.h"
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

void ShamelaUpdaterDialog::on_pushNext_clicked()
{
    switch(ui->stackedWidget->currentIndex()) {
    case 0:
        startUpdate();
        ui->stackedWidget->setCurrentIndex(1);
        ui->pushNext->setText(trUtf8("انتهى"));
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

void ShamelaUpdaterDialog::startUpdate()
{
    int progressMax = 7;

    // Progress dialog
    QProgressDialog progress(trUtf8("جاري تحديث فهرس %1...").arg(m_bookDb->indexInfo()->name()),
                             QString(),
                             0,
                             progressMax,
                             this);
    progress.setWindowModality(Qt::WindowModal);

    PROGRESS_DIALOG_STEP("التعرف على كتب الشاملة");

    QList<int> shaIds = m_bookDb->getShamelaIds();
    QList<int> savedIds = m_bookDb->getSavedIds();

    PROGRESS_DIALOG_STEP("مقارنة الكتب");

    if(shaIds.count() > savedIds.count())
        removeSameIds(shaIds, savedIds);
    else
        removeSameIds(savedIds, shaIds);

    if(shaIds.count() > 0  && ui->checkAddNewBooks->isChecked()) {

        PROGRESS_DIALOG_STEP("اضافة الكتب الجديدة الى قاعدة البيانات");
        QStringList addedBooksName = m_bookDb->addBooks(shaIds);

        PROGRESS_DIALOG_STEP("فهرسة الكتب الجديدة");
        indexBooks(shaIds, m_bookDb, m_bookDb->indexInfo());

        ui->labelAddedBooks->setText(trUtf8("تم اضافة %1:")
                                     .arg(arPlural(addedBooksName.count(), BOOK, true)));
        ui->listAddedBooks->addItems(addedBooksName);

        ui->widgetAddedBooks->show();
    } else {
        ui->widgetAddedBooks->hide();
    }

    if(savedIds.count() > 0 && ui->checkDeleteBooks->isChecked()) {

        PROGRESS_DIALOG_STEP("حذف الكتب من قاعدة البيانات");
        QStringList deletedBooksName = m_bookDb->removeBooks(savedIds);

        PROGRESS_DIALOG_STEP("حذف الكتب من الفهرس");
        deletBooksFromIndex(savedIds, m_bookDb->indexInfo());

        ui->labelDeletedBooks->setText(trUtf8("تم حذف %1:")
                                       .arg(arPlural(deletedBooksName.count(), BOOK, true)));
        ui->listDeletedBooks->addItems(deletedBooksName);
        ui->widgetDeletedBooks->show();
    } else {
        ui->widgetDeletedBooks->hide();
    }

    if(shaIds.count() > 0 || savedIds.count() > 0) {
        ui->labelNoChange->hide();
        ui->splitter->show();
    } else {
        ui->labelNoChange->show();
        ui->splitter->hide();
    }

    progress.setValue(progressMax);

}

void ShamelaUpdaterDialog::removeSameIds(QList<int> &bigList, QList<int> &smalList)
{
    int i=0;

    for(i=0; i < smalList.count(); i++) {
        int val = smalList.at(i);
        int index = bigList.indexOf(val);

        if(index != -1) {
            smalList.removeAt(i);
            bigList.removeAt(index);
            i--;
        }
    }
}

void ShamelaUpdaterDialog::indexBooks(QList<int> ids, BooksDB *bookDB, IndexInfo *info)
{
    try {
        IndexWriter *writer = NULL;
        QDir dir;
        ArabicAnalyzer *analyzer = new ArabicAnalyzer();
        if(!dir.exists(info->path()))
            dir.mkdir(info->path());
        if ( IndexReader::indexExists(qPrintable(info->path())) ){
            if ( IndexReader::isLocked(qPrintable(info->path())) ){
                IndexReader::unlock(qPrintable(info->path()));
            }

            writer = _CLNEW IndexWriter( qPrintable(info->path()), analyzer, false);
        } else {
            QMessageBox::critical(this,
                                  trUtf8("خطأ عند التحديث"),
                                  trUtf8("لم يتم العثور على اي فهرس في المسار" "\n" "%1").arg(info->path()));
            return;
        }

        writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
        writer->setRAMBufferSizeMB(info->ramSize());

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

void ShamelaUpdaterDialog::deletBooksFromIndex(QList<int> ids, IndexInfo *info)
{

    try {
        IndexWriter *writer = NULL;

        QDir dir;
        ArabicAnalyzer *analyzer = new ArabicAnalyzer();
        if(!dir.exists(info->path()))
            dir.mkdir(info->path());
        if ( IndexReader::indexExists(qPrintable(info->path())) ){
            if ( IndexReader::isLocked(qPrintable(info->path())) ){
                IndexReader::unlock(qPrintable(info->path()));
            }

            writer = _CLNEW IndexWriter( qPrintable(info->path()), analyzer, false);
        } else {
            QMessageBox::critical(this,
                                  trUtf8("خطأ عند التحديث"),
                                  trUtf8("لم يتم العثور على اي فهرس في المسار" "\n" "%1").arg(info->path()));
            return;
        }

        writer->setRAMBufferSizeMB(info->ramSize());

        for(int i=0; i<ids.count(); i++) {
            TCHAR str[10];
            Term *term = new Term(_itow(ids.at(i), str, 10), _T("bookid"));
            writer->deleteDocuments(term);
//            _CLDELETE(term);
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
