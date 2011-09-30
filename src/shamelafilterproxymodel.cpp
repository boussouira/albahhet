#include "shamelafilterproxymodel.h"
#include <qvariant.h>

ShamelaFilterProxyModel::ShamelaFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

ShamelaFilterProxyModel::~ShamelaFilterProxyModel()
{
}

bool ShamelaFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (filterAcceptsRowItself(source_row, source_parent))
        return true;

    //accept if any of the parents is accepted on it's own merits
    QModelIndex parent = source_parent;
    while (parent.isValid()) {
        if (filterAcceptsRowItself(parent.row(), parent.parent()))
            return true;
        parent = parent.parent();
    }

    //accept if any of the children is accepted on it's own merits
    if (hasAcceptedChildren(source_row, source_parent)) {
        return true;
    }

    return false;
}

bool ShamelaFilterProxyModel::filterAcceptsRowItself(int source_row, const QModelIndex &source_parent) const
{
    if(filterKeyColumn() == 2) {
        QModelIndex index = sourceModel()->index(source_row, 2, source_parent);
        int deathYear = index.data().toInt();

//        qDebug("%d <= %d <= %d", m_fromYear, deathYear, m_toYear);
        return (deathYear && m_fromYear <= deathYear && deathYear <= m_toYear);
    } else {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}

bool ShamelaFilterProxyModel::hasAcceptedChildren(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex item = sourceModel()->index(source_row,0,source_parent);
    if (!item.isValid()) {
        //qDebug() << "item invalid" << source_parent << source_row;
        return false;
    }

    //check if there are children
    int childCount = item.model()->rowCount(item);
    if (childCount == 0)
        return false;

    for (int i = 0; i < childCount; ++i) {
        if (filterAcceptsRowItself(i, item))
            return true;
        //recursive call
        if (hasAcceptedChildren(i, item))
            return true;
    }

    return false;
}

void ShamelaFilterProxyModel::setFilterByDeath(int fromYear, int toYear)
{
    m_fromYear = fromYear;
    m_toYear = toYear;
}
