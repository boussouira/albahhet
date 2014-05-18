#include "indexesdialog.h"
#include "ui_indexesdialog.h"
#include "common.h"
#include "cl_common.h"
#include "shamelaindexinfo.h"
#include "booksdb.h"
#include "shamelaupdaterdialog.h"
#include "indexesmanager.h"
#include "arabicanalyzer.h"
#include "indexoptimizer.h"

#include <qsettings.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qprogressdialog.h>
#include <qdir.h>
#include <assert.h>
#include <QTime>
#include <QProgressBar>

IndexesDialog::IndexesDialog(IndexesManager *manager, QWidget *parent) :
    m_indexesManager(manager),
    QDialog(parent),
    ui(new Ui::IndexesDialog),
    m_optimizeProgressdialog(0)
{
    ui->setupUi(this);
    hideWindowButtons(this);

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

    QList<IndexInfoBase *> list = m_indexesManager->list();
    QList<QTreeWidgetItem*> itemList;
    bool haveIndexes = !list.isEmpty();

    if(haveIndexes) {
        for(int i=0; i<list.count(); i++) {
            IndexInfoBase *info = list.at(i);

            QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
            item->setData(0, Qt::DisplayRole, info->name());
            item->setData(0, Qt::UserRole, info->id());

            itemList.append(item);
        }

        ui->treeWidget->addTopLevelItems(itemList);
    }
}

void IndexesDialog::indexOptimizeDone()
{
    IndexOptimizer *optimizer = qobject_cast<IndexOptimizer*>(sender());
    if(!optimizer)
        return;

    m_optimizeProgressdialog->close();

    QMessageBox::information(this,
                             tr("ضغط فهرس"),
                             tr("ثم ضغط الفهرس بنجاح خلال %1").arg(getTimeString(optimizer->optimizeTime())));
}

void IndexesDialog::on_pushEdit_clicked()
{
    IndexInfoBase *indexInfo = selectedIndex(true);
    if(!indexInfo) return;

    bool ok;
    QString text = QInputDialog::getText(this,
                                         tr("تغيير اسم الفهرس"),
                                         tr("الاسم الجديد:"),
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
                                 tr("تعديل فهرس"),
                                 tr("الاسم المدخل موجود مسبقا"));
        }
    }
}

void IndexesDialog::on_pushDelete_clicked()
{
    IndexInfoBase *indexInfo = selectedIndex(true);
    if(!indexInfo) return;

    int rep = QMessageBox::question(this,
                                    tr("حذف فهرس"),
                                    tr("هل تريد حذف فهرس <strong>%1</strong>؟").arg(indexInfo->name()),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if(rep == QMessageBox::Yes) {
        m_indexesManager->remove(indexInfo);
        loadIndexesList();
        emit indexesChanged();
    }
}

IndexInfoBase *IndexesDialog::selectedIndex(bool showWarningMsg)
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if(items.isEmpty()) {
        if(showWarningMsg) {
            QMessageBox::warning(this,
                                 tr("تعديل فهرس"),
                                 tr("لم تقم باختيار اي فهرس!"));
        }

        return 0;
    }

    int indexId = items.first()->data(0, Qt::UserRole).toInt();

    return m_indexesManager->indexInfo(indexId);
}

void IndexesDialog::on_pushUpDate_clicked()
{
    IndexInfoBase *indexInfo = selectedIndex(true);
    if(!indexInfo) return;

    BooksDB *bookDb = new BooksDB();
    bookDb->setIndexInfo(static_cast<ShamelaIndexInfo*>(indexInfo));

    ShamelaUpdaterDialog updater(this);
    updater.setBooksDB(bookDb);
    updater.resize(size());

    updater.exec();

    DELETE_DB(bookDb);
}

void IndexesDialog::on_pushOptimize_clicked()
{
    IndexInfoBase *indexInfo = selectedIndex(true);
    if(!indexInfo) return;

    int rep = QMessageBox::question(this,
                                    tr("ضغط فهرس"),
                                    tr("هل تريد ضغط فهرس <strong>%1</strong>؟"
                                       "<br>"
                                       "هذه العملية قد تأخذ بعض الوقت.")
                                    .arg(indexInfo->name()),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if(rep == QMessageBox::Yes){
        m_optimizeProgressdialog = new QProgressDialog(tr("جاري ضغط فهرس %1...").arg(indexInfo->name()),
                                                       QString(), 0, 1, this);
        m_optimizeProgressdialog->setWindowModality(Qt::WindowModal);

        QProgressBar *bar = new QProgressBar(m_optimizeProgressdialog);
        bar->setAlignment(Qt::AlignCenter);

        m_optimizeProgressdialog->setBar(bar);
        m_optimizeProgressdialog->setMaximum(0);

        hideWindowButtons(m_optimizeProgressdialog, true, true);

        optimizeIndex(indexInfo);

        m_optimizeProgressdialog->exec();
    }
}

void IndexesDialog::optimizeIndex(IndexInfoBase *info)
{

    IndexWriter *writer = 0;
    QDir dir;
    ArabicAnalyzer *analyzer = new ArabicAnalyzer();
    QSettings settings;

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
    writer->setRAMBufferSizeMB(settings.value("ramSize", 100).toInt());

    IndexOptimizer *optimizer = new IndexOptimizer(this);
    optimizer->setIndexWriter(writer);
    optimizer->setDeleteIndexWriter(true);

    connect(optimizer, SIGNAL(finished()), SLOT(indexOptimizeDone()));

    optimizer->start();
}

void IndexesDialog::on_treeWidget_itemActivated(QTreeWidgetItem* , int column)
{
    if(column != -1) {
        IndexInfoBase *indexInfo = selectedIndex();
        if(indexInfo) {
            ui->labelIndexName->setText(indexInfo->name());
            ui->labelShaPath->setText(static_cast<ShamelaIndexInfo*>(indexInfo)->shamelaPath());
            ui->labelIndexPath->setText(indexInfo->path());

            ui->widgetIndexInfo->show();
        }
    } else {
        ui->widgetIndexInfo->hide();
    }
}

void IndexesDialog::setIndexesManager(IndexesManager *manager)
{
    m_indexesManager = manager;
}

void IndexesDialog::on_treeWidget_itemSelectionChanged()
{
    on_treeWidget_itemActivated(ui->treeWidget->currentItem(), ui->treeWidget->currentColumn());
}
