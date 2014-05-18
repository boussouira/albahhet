#ifndef ABSTRACTSEARCHWIDGET_H
#define ABSTRACTSEARCHWIDGET_H

#include <QWidget>

class IndexInfoBase;
class BooksDB;
class ShamelaModels;
class SearchFilterHandler;
class TabWidget;

class AbstractSearchWidget : public QWidget
{
    Q_OBJECT

public:
    AbstractSearchWidget(QWidget *parent = 0);
    ~AbstractSearchWidget();

    virtual void setIndexInfo(IndexInfoBase *info)=0;
    virtual void setTabWidget(TabWidget *tabWidget)=0;
    virtual void indexChanged()=0;
    virtual void saveSettings()=0;
    virtual QList<int> selectedBooks();
    virtual void selectBooks(QList<int> books);

public slots:
    virtual void selectAllBooks() {}
    virtual void unSelectAllBooks() {}
    virtual void selectVisibleBooks() {}
    virtual void unSelectVisibleBooks() {}
    virtual void expandFilterView() {}
    virtual void collapseFilterView() {}
};

#endif // ABSTRACTSEARCHWIDGET_H
