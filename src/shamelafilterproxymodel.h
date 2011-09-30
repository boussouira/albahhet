#ifndef SHAMELAFILTERPROXYMODEL_H
#define SHAMELAFILTERPROXYMODEL_H

#include <qsortfilterproxymodel.h>
#include "booksdb.h"

class ShamelaFilterProxyModel : public QSortFilterProxyModel
{
public:
    ShamelaFilterProxyModel(QObject *parent = 0);
    ~ShamelaFilterProxyModel();
    void setFilterByDeath(int fromYear, int toYear);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const;
    bool hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const;

protected:
    int m_fromYear;
    int m_toYear;
};

#endif // SHAMELAFILTERPROXYMODEL_H
