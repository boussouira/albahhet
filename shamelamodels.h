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
    void setIndexInfo(IndexInfo *info);
    void setBooksListModel(QStandardItemModel *model);
    void setCatsListModel(QStandardItemModel *model);
    void setAuthorsListModel(QStandardItemModel *model);

    QList<int> getSelectedBooks();
    QList<int> getSelectedCats();
    QList<int> getSelectedAuthors();

    QStandardItemModel *booksModel() { return m_booksModel; }
    QStandardItemModel *catsModel() { return m_catsModel; }
    QStandardItemModel *authorsModel() { return m_authorsModel; }

     QStandardItemModel *getModel(int index);
public slots:
    void booksListChange(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/);
    void catsListChange(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/);
    void authorsListChange(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/);

protected:
    IndexInfo *m_indexInfo;
    QStandardItemModel *m_booksModel;
    QStandardItemModel *m_catsModel;
    QStandardItemModel *m_authorsModel;
    QList<int> m_bookIds;
    QList<int> m_catIds;
    QList<int> m_authorIds;
    bool m_reload;

};

#endif // SHAMELAMODELS_H
