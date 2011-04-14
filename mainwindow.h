#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>
#include <qhash.h>
#include "indexesmanager.h"

namespace Ui {
    class MainWindow;
}

class IndexInfo;
class BooksDB;
class TabWidget;
class ShamelaSearchWidget;
class LogDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void selectIndex(int id);
    void selectIndex(QAction *action);
    void indexChanged();
    void loadIndexesList();
    void haveIndexesCheck();

protected:
    void closeEvent(QCloseEvent *e);

protected slots:
    void saveSettings();
    void loadSettings();
    void changeIndex();
    void updateIndexesMenu();
    void newIndex();
    void editIndexes();
    void showStatistic();
    void aboutApp();
    void showLogDialog();

protected:
    IndexesManager *m_indexesManager;
    TabWidget *m_tabWidget;
    ShamelaSearchWidget *m_searchWidget;
    LogDialog *m_logDialog;
    IndexInfo *m_currentIndex;
    BooksDB *m_booksDB;
    bool m_showNewIndexMsg;
    Ui::MainWindow *ui;

private slots:
    void showSettingsDialog();
};

#endif // MAINWINDOW_H
