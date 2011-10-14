#ifndef SHAMELASELECTBOOKDIALOG_H
#define SHAMELASELECTBOOKDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "booksdb.h"
#include "shamelafilterproxymodel.h"

namespace Ui {
    class ShamelaSelectBookDialog;
}

class ShamelaSelectBookDialog : public QDialog
{
    Q_OBJECT

public:
    ShamelaSelectBookDialog(BooksDB *db, QWidget *parent = 0);
    ~ShamelaSelectBookDialog();

    void loadbooksList();

    int selectedBookID;
    int selectedBookVersion;
    QString selectedBookName;

protected slots:
    void selectBook();
    void cancel();

private:
    Ui::ShamelaSelectBookDialog *ui;
    BooksDB *m_booksDB;
    QStandardItemModel *m_model;
    ShamelaFilterProxyModel *m_filter;
};

#endif // SHAMELASELECTBOOKDIALOG_H
