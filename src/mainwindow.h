#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>
#include <qhash.h>
#include "indexesmanager.h"
#include "updatechecker.h"
#include "supportdialog.h"

namespace Ui {
    class MainWindow;
}

class IndexInfo;
class BooksDB;
class TabWidget;
class AbstractSearchWidget;
class LogDialog;
class SearchField;

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
    void loadSearchFields();

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
    void saveSelectedField();
    void searchfieldSelected();
    void searchfieldsDialog();
    void showSettingsDialog();
    void checkFinnished();
    void autoUpdateCheck();

protected:
    IndexesManager *m_indexesManager;
    TabWidget *m_tabWidget;
    AbstractSearchWidget *m_searchWidget;
    LogDialog *m_logDialog;
    IndexInfo *m_currentIndex;
    BooksDB *m_booksDB;
    SearchField *m_searchFields;
    UpdateChecker *m_updateChecker;
    SupportDialog m_supportDialog;
    bool m_showNewIndexMsg;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
