#include "shamelamodels.h"
#include "common.h"

ShamelaModels::ShamelaModels(QObject *parent): QObject(parent)
{
    m_reload = true;
}

void ShamelaModels::setIndexInfo(ShamelaIndexInfo *info)
{
    m_reload = true;
    m_indexInfo = info;
}

void ShamelaModels::setBooksListModel(QStandardItemModel *model)
{
    m_booksModel = model;
    m_selectedBooks.clear();
    m_unSelectedBooks.clear();
}

QList<int> ShamelaModels::selectedBooks()
{
    return m_selectedBooks;
}

QList<int> ShamelaModels::unSelectedBooks()
{
    return m_unSelectedBooks;
}

void ShamelaModels::getBooks(QModelIndex index)
{
    if(index.isValid()) {
        QModelIndex child = index.child(0, 0);
        while(child.isValid()) {
            if(child.data(BooksDB::typeRole).toInt() == BooksDB::Book) {
                if(child.data(Qt::CheckStateRole).toInt() ==  Qt::Checked) {
                    m_selectedBooks.append(child.data(BooksDB::idRole).toInt());
                } else {
                    m_unSelectedBooks.append(child.data(BooksDB::idRole).toInt());
                }
            }

            child = index.child(child.row()+1, 0);
        }
    }
}

void ShamelaModels::generateLists()
{
    m_selectedBooks.clear();
    m_unSelectedBooks.clear();

    QModelIndex index = m_booksModel->index(0, 0);
    while(index.isValid()) {
        getBooks(index);

        index = index.sibling(index.row()+1, 0);
    }
}

int ShamelaModels::selectedBooksCount()
{
    return m_selectedBooks.count();
}

int ShamelaModels::unSelectBooksCount()
{
    return m_unSelectedBooks.count();
}
