#ifndef INDEXESDIALOG_H
#define INDEXESDIALOG_H

#include <qdialog.h>
#include <qhash.h>

namespace Ui {
    class IndexesDialog;
}

class IndexInfoBase;
class BooksDB;
class QTreeWidgetItem;
class IndexesManager;
class QProgressDialog;

class IndexesDialog : public QDialog
{
    Q_OBJECT

public:
    IndexesDialog(IndexesManager *manager, QWidget *parent = 0);
    ~IndexesDialog();
    void setIndexesManager(IndexesManager *manager);
    void optimizeIndex(IndexInfoBase *info);

protected slots:
    IndexInfoBase *selectedIndex(bool showWarningMsg=false);
    void loadIndexesList();
    void indexOptimizeDone();

signals:
    void indexesChanged();

private slots:
    void on_treeWidget_itemSelectionChanged();
    void on_treeWidget_itemActivated(QTreeWidgetItem*, int column);
    void on_pushOptimize_clicked();
    void on_pushUpDate_clicked();
    void on_pushDelete_clicked();
    void on_pushEdit_clicked();

private:
    Ui::IndexesDialog *ui;
    IndexesManager *m_indexesManager;
    QProgressDialog *m_optimizeProgressdialog;
};

#endif // INDEXESDIALOG_H
