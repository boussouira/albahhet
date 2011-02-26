#ifndef INDEXESDIALOG_H
#define INDEXESDIALOG_H

#include <qdialog.h>
#include <qhash.h>

namespace Ui {
    class IndexesDialog;
}

class IndexInfo;
class BooksDB;
class QTreeWidgetItem;

class IndexesDialog : public QDialog
{
    Q_OBJECT

public:
    IndexesDialog(QWidget *parent = 0);
    ~IndexesDialog();
    bool changeIndexName(IndexInfo *index, QString newName);
    bool deleteIndex(IndexInfo *index);
    void optimizeIndex(IndexInfo *info);

protected slots:
    void loadIndexesList();

signals:
    void indexesChanged();

private:
    Ui::IndexesDialog *ui;
    QHash<QString, IndexInfo*> m_indexInfoMap;

private slots:
    void on_treeWidget_itemActivated(QTreeWidgetItem* item, int column);
    void on_pushOptimize_clicked();
    void on_pushUpDate_clicked();
    void on_pushDelete_clicked();
    void on_pushEdit_clicked();
};

#endif // INDEXESDIALOG_H
