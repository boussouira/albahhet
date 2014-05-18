#ifndef SHAMELAMODELS_H
#define SHAMELAMODELS_H

#include "booksdb.h"
#include <qobject.h>
#include <qabstractitemmodel.h>
#include <qstandarditemmodel.h>

class ShamelaModels : public QObject
{
    Q_OBJECT

public:
    ShamelaModels(QObject *parent = 0);
    void setIndexInfo(ShamelaIndexInfo *info);
    void setBooksListModel(QStandardItemModel *model);

    QList<int> selectedBooks();
    QList<int> unSelectedBooks();
    int selectedBooksCount();
    int unSelectBooksCount();

    void generateLists();
    QStandardItemModel *booksModel() { return m_booksModel; }

    void getBooks(QModelIndex index);

protected:
    ShamelaIndexInfo *m_indexInfo;
    QStandardItemModel *m_booksModel;
    QList<int> m_selectedBooks;
    QList<int> m_unSelectedBooks;
    bool m_reload;
};

#endif // SHAMELAMODELS_H
