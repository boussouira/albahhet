#include "shamelamodels.h"
#include "common.h"

ShamelaModels::ShamelaModels(QObject *parent): QObject(parent)
{
    m_reload = true;
}

void ShamelaModels::setIndexInfo(IndexInfo *info)
{
    m_reload = true;
    m_indexInfo = info;
}

void ShamelaModels::setBooksListModel(QStandardItemModel *model)
{
    m_booksModel = model;
    m_bookIds.clear();

    connect(model,
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(booksListChange(QModelIndex,QModelIndex)));
}

void ShamelaModels::setCatsListModel(QStandardItemModel *model)
{
    m_catsModel = model;
    m_catIds.clear();

    connect(model,
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(catsListChange(QModelIndex,QModelIndex)));
}

void ShamelaModels::setAuthorsListModel(QStandardItemModel *model)
{
    m_authorsModel = model;
    m_authorIds.clear();

    connect(model,
            SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(authorsListChange(QModelIndex,QModelIndex)));
}

void ShamelaModels::booksListChange(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/)
{
    int id = topLeft.data(Qt::UserRole).toInt();
    bool checked = topLeft.data(Qt::CheckStateRole).toBool();

    if(checked)
        m_bookIds.append(id);
    else
        m_bookIds.removeAll(id);
}

void ShamelaModels::catsListChange(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/)
{
    int id = topLeft.data(Qt::UserRole).toInt();
    bool checked = topLeft.data(Qt::CheckStateRole).toBool();

    if(checked)
        m_catIds.append(id);
    else
        m_catIds.removeAll(id);
}

void ShamelaModels::authorsListChange(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/)
{
    int id = topLeft.data(Qt::UserRole).toInt();
    bool checked = topLeft.data(Qt::CheckStateRole).toBool();

    if(checked)
        m_authorIds.append(id);
    else
        m_authorIds.removeAll(id);
}

QList<int> ShamelaModels::getSelectedBooks()
{
    return m_bookIds;
}

QList<int> ShamelaModels::getSelectedCats()
{
    return m_catIds;
}

QList<int> ShamelaModels::getSelectedAuthors()
{
    return m_authorIds;
}

QStandardItemModel * ShamelaModels::getModel(int index)
{
    if(index == 0)
        return m_booksModel;
    else if(index == 1)
        return m_catsModel;
    else if(index == 2)
        return m_authorsModel;
    else
        return 0;
}

