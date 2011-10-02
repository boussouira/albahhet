#ifndef SHAMELASEARCHWIDGET_H
#define SHAMELASEARCHWIDGET_H

#include "abstractsearchwidget.h"
#include "arabicanalyzer.h"
#include <QModelIndex>
#include <QStandardItem>

namespace Ui {
    class ShamelaSearchWidget;
}

class IndexInfo;
class BooksDB;
class ShamelaModels;
class SearchFilterHandler;
class TabWidget;

class ShamelaSearchWidget : public AbstractSearchWidget
{
    Q_OBJECT

public:
    ShamelaSearchWidget(QWidget *parent = 0);
    ~ShamelaSearchWidget();
    void setIndexInfo(IndexInfo *info);
    void setBooksDb(BooksDB *db);
    void setTabWidget(TabWidget *tabWidget);
    void saveSettings();
    void indexChanged();

protected:
    void closeEvent(QCloseEvent *e);
    Query *getBooksListQuery();
    void setupCleanMenu();

public slots:
    void search();
    void loadSettings();
    void clearSpecialChar();
    void clearLineText();

protected slots:
    void on_lineFilter_textChanged(QString text);
    void on_pushSelectAll_clicked();
    void on_pushUnSelectAll_clicked();
    void itemChanged(QStandardItem *item);

protected:
    IndexInfo *m_currentIndex;
    BooksDB *m_booksDB;
    TabWidget *m_tabWidget;
    ShamelaModels *m_shaModel;
    SearchFilterHandler *m_filterHandler;
    QString m_searchQuery;
    int m_resultParPage;
    int m_searchCount;
    bool m_useMultiTab;
    bool m_proccessItemChange;

private slots:
    void on_pushExpandTree_clicked();

    void on_pushCollapseTree_clicked();

private:
    Ui::ShamelaSearchWidget *ui;
};

#endif // SHAMELASEARCHWIDGET_H
