#include "indexesdialog.h"
#include "ui_indexesdialog.h"
#include "common.h"
#include "cl_common.h"
#include "indexinfo.h"
#include "booksdb.h"
#include "indexingdialg.h"
#include "shamelaupdaterdialog.h"
#include "indexesmanager.h"
#include <qsettings.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qprogressdialog.h>
#include <qdir.h>
#include <assert.h>

IndexesDialog::IndexesDialog(IndexesManager *manager, QWidget *parent) :
    m_indexesManager(manager),
    QDialog(parent),
    ui(new Ui::IndexesDialog)
{
    ui->setupUi(this);
    hideHelpButton(this);

    loadIndexesList();
    ui->widgetIndexInfo->hide();
}

IndexesDialog::~IndexesDialog()
{
    delete ui;
}

void IndexesDialog::loadIndexesList()
{
    ui->treeWidget->clear();
    m_indexesManager->clear();

    QList<IndexInfo *> list = m_indexesManager->list();
    QList<QTreeWidgetItem*> itemList;
    bool haveIndexes = !list.isEmpty();

    if(haveIndexes) {
        for(int i=0; i<list.count(); i++) {
            IndexInfo *info = list.at(i);

            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
            item->setData(0, Qt::DisplayRole, info->name());
            item->setData(0, Qt::UserRole, info->id());

            itemList.append(item);
        }

        ui->treeWidget->addTopLevelItems(itemList);
    }
}

void IndexesDialog::on_pushEdit_clicked()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if(items.count() > 0) {
        bool ok;
        IndexInfo *indexInfo = m_indexesManager->indexInfo(items.at(0)->data(0, Qt::UserRole).toInt());

        QString text = QInputDialog::getText(this,
                                             trUtf8("تغيير اسم الفهرس"),
                                             trUtf8("الاسم الجديد:"),
                                             QLineEdit::Normal,
                                             indexInfo->name(),
                                             &ok);
        if (ok && !text.isEmpty() && text != indexInfo->name()) {
            if(!m_indexesManager->nameExists(text)) {
                m_indexesManager->setIndexName(indexInfo, text);
                loadIndexesList();
                emit indexesChanged();

            } else {
                QMessageBox::warning(this,
                                     trUtf8("تعديل فهرس"),
                                     trUtf8("الاسم المدخل موجود مسبقا"));
            }
        }

    } else {
        QMessageBox::warning(this,
                             trUtf8("تعديل فهرس"),
                             trUtf8("لم تقم باختيار اي فهرس!"));
    }
}

void IndexesDialog::on_pushDelete_clicked()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if(items.count() > 0) {
        IndexInfo *indexInfo = m_indexesManager->indexInfo(items.at(0)->data(0, Qt::UserRole).toInt());
        int rep = QMessageBox::question(this,
                                        trUtf8("حذف فهرس"),
                                        trUtf8("هل تريد حذف فهرس <strong>%1</strong>؟").arg(indexInfo->name()),
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

        if(rep == QMessageBox::Yes) {
            m_indexesManager->remove(indexInfo);
            loadIndexesList();
            emit indexesChanged();
        }
    } else {
        QMessageBox::warning(this,
                             trUtf8("حذف فهرس"),
                             trUtf8("لم تقم باختيار اي فهرس!"));
    }
}

void IndexesDialog::on_pushUpDate_clicked()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();

    if(items.count() > 0) {
        IndexInfo *indexInfo = m_indexesManager->indexInfo(items.at(0)->data(0, Qt::UserRole).toInt());

        BooksDB *bookDb = new BooksDB();
        bookDb->setIndexInfo(indexInfo);

        ShamelaUpdaterDialog updater(this);
        updater.setBooksDB(bookDb);
        updater.resize(size());

        updater.exec();

        DELETE_DB(bookDb);

    } else {
        QMessageBox::warning(this,
                             trUtf8("تحديث فهرس"),
                             trUtf8("لم تقم باختيار اي فهرس!"));
    }
}

void IndexesDialog::on_pushOptimize_clicked()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if(items.count() > 0) {
        IndexInfo *indexInfo = m_indexesManager->indexInfo(items.at(0)->data(0, Qt::UserRole).toInt());
        int rep = QMessageBox::question(this,
                             trUtf8("ضغط فهرس"),
                             trUtf8("هل تريد ضغط فهرس <strong>%1</strong>؟"
                                    "<br>"
                                    "هذه العملية قد تأخذ بعض الوقت وقد يتجمد البرنامج قليلا.")
                             .arg(indexInfo->name()),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

        if(rep == QMessageBox::Yes){
            QProgressDialog progress(trUtf8("جاري ضغط فهرس %1...").arg(indexInfo->name()),
                                     QString(),
                                     0,
                                     1,
                                     this);
            progress.setWindowModality(Qt::WindowModal);
            progress.setMinimumDuration(0);

            optimizeIndex(indexInfo);

            progress.setValue(1);

            QMessageBox::information(this,
                                     trUtf8("ضغط فهرس"),
                                     trUtf8("ثم ضغط الفهرس بنجاح"));
        }

    } else {
        QMessageBox::warning(this,
                             trUtf8("ضغط فهرس"),
                             trUtf8("لم تقم باختيار اي فهرس!"));
    }
}

void IndexesDialog::optimizeIndex(IndexInfo *info)
{
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

    writer->setUseCompoundFile(false);

    writer->optimize(MAX_SEGMENT);
    writer->close();

    _CLDELETE(writer);
}

void IndexesDialog::on_treeWidget_itemActivated(QTreeWidgetItem* item, int column)
{
    if(column != -1) {
        IndexInfo *indexInfo = m_indexesManager->indexInfo(item->data(0, Qt::UserRole).toInt());
        ui->labelIndexName->setText(indexInfo->name());
        ui->labelShaPath->setText(indexInfo->shamelaPath());
        ui->labelIndexPath->setText(indexInfo->path());

        ui->widgetIndexInfo->show();
    } else {
        ui->widgetIndexInfo->hide();
    }
}

void IndexesDialog::setIndexesManager(IndexesManager *manager)
{
    m_indexesManager = manager;
}
