#ifndef SHAMELAFILTERPROXYMODEL_H
#define SHAMELAFILTERPROXYMODEL_H

#include <qsortfilterproxymodel.h>
#include "booksdb.h"

class ShamelaFilterProxyModel : public QSortFilterProxyModel
{
public:
    ShamelaFilterProxyModel(QObject *parent = 0);
    ~ShamelaFilterProxyModel();
    void setFilterByAuthor(bool authorFilter);
    void setAuthor(int id);
    void setBooksDb(BooksDB *db) { m_booksDB = db;}

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const;
    bool hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const;
    BooksDB *m_booksDB;
    QList<int> m_list;
};

#endif // SHAMELAFILTERPROXYMODEL_H
