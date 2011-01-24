#ifndef INDEXESDIALOG_H
#define INDEXESDIALOG_H

#include <qdialog.h>
#include <qhash.h>

namespace Ui {
    class IndexesDialog;
}

class IndexInfo;
class BooksDB;

class IndexesDialog : public QDialog
{
    Q_OBJECT

public:
    IndexesDialog(QWidget *parent = 0);
    ~IndexesDialog();
    bool changeIndexName(IndexInfo *index, QString newName);
    bool deleteIndex(IndexInfo *index);
    void removeSameIds(QList<int> &big, QList<int> &small);
    void indexBooks(QList<int> ids, BooksDB *bookDB, IndexInfo *info);
    void deletBooksFromIndex(QList<int> ids, IndexInfo *info);
    void optimizeIndex(IndexInfo *info);

protected slots:
    void loadIndexesList();

signals:
    void indexesChanged();

private:
    Ui::IndexesDialog *ui;
    QHash<QString, IndexInfo*> m_indexInfoMap;

private slots:
    void on_pushOptimize_clicked();
    void on_pushUpDate_clicked();
    void on_pushDelete_clicked();
    void on_pushEdit_clicked();
};

#endif // INDEXESDIALOG_H
