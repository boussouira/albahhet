#ifndef ABSTRACTSEARCHWIDGET_H
#define ABSTRACTSEARCHWIDGET_H

#include <QWidget>

class IndexInfo;
class BooksDB;
class ShamelaModels;
class SearchFilterHandler;
class TabWidget;

class AbstractSearchWidget : public QWidget
{
    Q_OBJECT

public:
    AbstractSearchWidget(QWidget *parent = 0);
    virtual ~AbstractSearchWidget();

    virtual void setIndexInfo(IndexInfo *info)=0;
    virtual void setBooksDb(BooksDB *db)=0;
    virtual void setTabWidget(TabWidget *tabWidget)=0;
    virtual void indexChanged()=0;
    virtual void saveSettings()=0;

};

#endif // ABSTRACTSEARCHWIDGET_H
