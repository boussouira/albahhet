#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>
#include <qsqldatabase.h>
#include <qhash.h>
#include <qabstractitemmodel.h>
#include <qsortfilterproxymodel.h>
#include "cl_common.h"
#include "searchfilterhandler.h"

namespace Ui {
    class MainWindow;
}

class IndexInfo;
class BooksDB;
class ShamelaModels;
class QSqlQuery;
class QProgressBar;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void selectIndex(QString name);
    void selectIndex(QAction *action);
    void indexChanged();
    void loadIndexesList();
    void haveIndexesCheck();

protected:
    void closeEvent(QCloseEvent *e);
    QString getIndexSize();
    QString getBooksSize();
    qint64 getDirSize(const QString &path);
    void doneIndexing(int indexingTime);
    Query *getBooksListQuery();
    Query *getCatsListQuery();
    Query *getAuthorsListQuery();
    void chooseProxy(int index);

protected slots:
    void saveSettings();
    void loadSettings();
    void changeIndex();
    void updateIndexesMenu();
    void newIndex();
    void editIndexes();
    void startSearching();
    void showStatistic();
    void resultsCount();
    bool openDB();
    void tabCountChange(int count);
    void closeTab(int index);
    QString buildFilePath(QString bkid, int archive);
    void aboutApp();


protected:
    IndexInfo *m_currentIndex;
    BooksDB *m_booksDB;
    ShamelaModels *m_shaModel;
    SearchFilterHandler *m_filterHandler;
    QSqlDatabase m_bookDB;
    QSqlQuery *m_bookQuery;
    QString m_titleName;
    QString m_bookTableName;
    QString m_searchQuery;
    QString m_highLightRE;
    QHash<QString, IndexInfo*> m_indexInfoMap;
    QStringList m_filterText;
    int m_resultParPage;
    int m_searchCount;
    bool m_dbIsOpen;
    bool m_haveMainTable;
    bool m_useMultiTab;
    bool m_showNewIndexMsg;
    Ui::MainWindow *ui;

private slots:
    void on_tabWidgetFilter_currentChanged(int index);
    void on_lineFilter_textChanged(QString text);
    void showSettingsDialog();
    void on_lineQueryMust_returnPressed();
    void on_lineQueryShould_returnPressed();
    void on_lineQueryShouldNot_returnPressed();
};

#endif // MAINWINDOW_H
