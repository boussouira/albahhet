#include "indexesdialog.h"
#include "ui_indexesdialog.h"
#include "common.h"
#include "cl_common.h"
#include "indexinfo.h"
#include "booksdb.h"
#include "indexingdialg.h"
#include "shamelaupdaterdialog.h"
#include <qsettings.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qprogressdialog.h>
#include <qdir.h>
#include <assert.h>

IndexesDialog::IndexesDialog(QWidget *parent) :
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
    qDeleteAll(m_indexInfoMap);
}

bool IndexesDialog::changeIndexName(IndexInfo *index, QString newName)
{
    QSettings settings;
    QStringList list =  settings.value("indexes_list").toStringList();

    QString oldHash = index->indexHash();
    QString newHash = IndexInfo::indexHash(newName);

    int oldIndex = list.indexOf(oldHash);
    if(oldIndex != -1) {

        settings.beginGroup(newHash);
        settings.setValue("name", newName);
        settings.setValue("shamela_path", index->shamelaPath());
        settings.setValue("index_path", index->path());
        settings.setValue("ram_size", index->ramSize());
        settings.setValue("optimizeIndex", index->optimize());
        settings.endGroup();

        list.replace(oldIndex, newHash);
        settings.setValue("indexes_list", list);

        settings.remove(oldHash);

        if(oldHash == settings.value("current_index").toString())
            settings.setValue("current_index", newHash);

        return true;
    }

    return false;
}

bool IndexesDialog::deleteIndex(IndexInfo *index)
{
    QSettings settings;
    QStringList list =  settings.value("indexes_list").toStringList();
    QString indexHash = index->indexHash();

    int indexIndex = list.indexOf(indexHash);

    if(indexIndex != -1) {
        settings.remove(indexHash);
        list.removeAt(indexIndex);

        if(list.isEmpty())
            settings.remove("indexes_list");
        else
            settings.setValue("indexes_list", list);

        if(indexHash == settings.value("current_index").toString()) {
            if(list.isEmpty())
                settings.remove("current_index");
            else
                settings.setValue("current_index", list.first());
        }
        return true;
    }

    return false;
}

void IndexesDialog::loadIndexesList()
{
    ui->treeWidget->clear();

    QSettings settings;
    QStringList list =  settings.value("indexes_list").toStringList();
    QList<QTreeWidgetItem*> itemList;
    bool haveIndexes = !list.isEmpty();

    if(haveIndexes) {
//        QString current = settings.value("current_index", list.first()).toString();

        for(int i=0; i<list.count(); i++) {
            IndexInfo *info = new IndexInfo();
            QString indexHash(list.at(i));

            settings.beginGroup(indexHash);
            info->setName(settings.value("name").toString());
            info->setShamelaPath(settings.value("shamela_path").toString());
            info->setPath(settings.value("index_path").toString());
            info->setRamSize(settings.value("ram_size").toInt());
            info->setOptimizeIndex(settings.value("optimizeIndex").toBool());
            settings.endGroup();

            m_indexInfoMap.insert(indexHash, info);

            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
            item->setData(0, Qt::DisplayRole, info->name());
            item->setData(0, Qt::UserRole, indexHash);
/*
            if(current == indexHash) {
                item->setData(0,
                              Qt::ForegroundRole,
                              QBrush(Qt::green));
            }
*/
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
        IndexInfo *indexInfo = m_indexInfoMap[items.at(0)->data(0, Qt::UserRole).toString()];

        QString text = QInputDialog::getText(this,
                                             trUtf8("تغيير اسم الفهرس"),
                                             trUtf8("الاسم الجديد:"),
                                             QLineEdit::Normal,
                                             indexInfo->name(),
                                             &ok);
        if (ok && !text.isEmpty() && text != indexInfo->name()) {
            if(!m_indexInfoMap.keys().contains(IndexInfo::indexHash(text))) {
                if(changeIndexName(indexInfo, text)) {
                    loadIndexesList();
                    emit indexesChanged();
                }
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
        IndexInfo *indexInfo = m_indexInfoMap[items.at(0)->data(0, Qt::UserRole).toString()];
        int rep = QMessageBox::question(this,
                             trUtf8("حذف فهرس"),
                             trUtf8("هل تريد حذف فهرس <strong>%1</strong>؟").arg(indexInfo->name()),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

        if(rep == QMessageBox::Yes) {
            if(deleteIndex(indexInfo)) {
                loadIndexesList();
                emit indexesChanged();
            }
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
        IndexInfo *indexInfo = m_indexInfoMap[items.at(0)->data(0, Qt::UserRole).toString()];

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
        IndexInfo *indexInfo = m_indexInfoMap[items.at(0)->data(0, Qt::UserRole).toString()];
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

    writer->optimize();
    writer->close();

    _CLDELETE(writer);
}

void IndexesDialog::on_treeWidget_itemActivated(QTreeWidgetItem* item, int column)
{
    if(column != -1) {
        IndexInfo *indexInfo = m_indexInfoMap[item->data(0, Qt::UserRole).toString()];
        ui->labelIndexName->setText(indexInfo->name());
        ui->labelShaPath->setText(indexInfo->shamelaPath());
        ui->labelIndexPath->setText(indexInfo->path());

        ui->widgetIndexInfo->show();
    } else {
        ui->widgetIndexInfo->hide();
    }
}
