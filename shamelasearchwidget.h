#ifndef SHAMELASEARCHWIDGET_H
#define SHAMELASEARCHWIDGET_H

#include <QWidget>
#include <QModelIndex>
#include "arabicanalyzer.h"

namespace Ui {
    class ShamelaSearchWidget;
}

class IndexInfo;
class BooksDB;
class ShamelaModels;
class SearchFilterHandler;
class TabWidget;

class ShamelaSearchWidget : public QWidget
{
    Q_OBJECT

public:
    ShamelaSearchWidget(QWidget *parent = 0);
    ~ShamelaSearchWidget();
    void setIndexInfo(IndexInfo *info);
    void setBooksDb(BooksDB *db);
    void setTabWidget(TabWidget *tabWidget);
    void indexChanged();

protected:
    void closeEvent(QCloseEvent *e);
    Query *getBooksListQuery();
    Query *getCatsListQuery();
    Query *getAuthorsListQuery();
    void chooseProxy(int index);
    void setupCleanMenu();

public slots:
    void search();
    void loadSettings();
    void saveSettings();
    void clearSpecialChar();
    void clearLineText();
    void enableFilterWidget();

protected slots:
    void on_tabWidgetFilter_currentChanged(int index);
    void on_lineFilter_textChanged(QString text);
    void on_treeViewAuthors_doubleClicked(QModelIndex index);
    void on_pushSelectAll_clicked();
    void on_pushUnSelectAll_clicked();

protected:
    IndexInfo *m_currentIndex;
    BooksDB *m_booksDB;
    ShamelaModels *m_shaModel;
    SearchFilterHandler *m_filterHandler;
    TabWidget *m_tabWidget;
    QString m_searchQuery;
    QStringList m_filterText;
    int m_resultParPage;
    int m_searchCount;
    bool m_useMultiTab;

private:
    Ui::ShamelaSearchWidget *ui;
};

#endif // SHAMELASEARCHWIDGET_H
