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
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterByAuthor() { return m_filterByAuthor; }

protected:
    bool m_filterByAuthor;
    int m_ahuthorId;
    BooksDB *m_booksDB;
    QList<int> m_list;
};

#endif // SHAMELAFILTERPROXYMODEL_H
