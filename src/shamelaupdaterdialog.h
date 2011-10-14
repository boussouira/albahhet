#ifndef SHAMELAUPDATERDIALOG_H
#define SHAMELAUPDATERDIALOG_H

#include <QDialog>
#include "shamelaupdater.h"

namespace Ui {
    class ShamelaUpdaterDialog;
}

class BooksDB;
class IndexInfo;
class ShamelaUpdater;

class ShamelaUpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    ShamelaUpdaterDialog(QWidget *parent = 0);
    ~ShamelaUpdaterDialog();
    void setBooksDB(BooksDB *db);

protected:
    void getUpdateBooks();
    void startUpdate();
    void indexBooks(QList<int> ids, BooksDB *bookDB, IndexInfo *info);
    void deletBooksFromIndex(QList<int> ids, IndexInfo *info);

private:
    Ui::ShamelaUpdaterDialog *ui;
    BooksDB *m_bookDb;
    ShamelaUpdater m_updater;
    QStandardItemModel *m_model;
    QList<ShamelaUpdaterTask> m_addedTasks;

private slots:
    void on_pushCancel_clicked();
    void on_pushNext_clicked();
    void on_toolAdd_clicked();
    void on_toolDelete_clicked();
};

#endif // SHAMELAUPDATERDIALOG_H
