#include "shamelafilterproxymodel.h"
#include <qvariant.h>

ShamelaFilterProxyModel::ShamelaFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      m_filterByAuthor(false)
{
}

ShamelaFilterProxyModel::~ShamelaFilterProxyModel()
{
}


void ShamelaFilterProxyModel::setFilterByAuthor(bool authorFilter)
{
    m_filterByAuthor = authorFilter;
}

bool ShamelaFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(m_filterByAuthor) {
        int bid = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole).toInt();
        return m_list.contains(bid);
    } else {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}

void ShamelaFilterProxyModel::setAuthor(int id)
{
    m_ahuthorId = id;
    m_list = m_booksDB->getAuthorBooks(m_ahuthorId);
}
