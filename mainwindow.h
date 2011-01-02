#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qstandarditemmodel.h>
#include <qaction.h>
#include <qhash.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include "qwebframe.h"
#include "qtreewidget.h"

#include "common.h"
#include "arabicanalyzer.h"
#include "indexingdialg.h"
#include "indexthread.h"
#include "shamelasearcher.h"
#include "shamelaresultwidget.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void loadIndexesList();
    void selectIndex(QString name);
    void selectIndex(QAction *act);
    void indexChanged();
    void updateIndexesMenu();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);
    QString getIndexSize();
    QString getBooksSize();
    qint64 getDirSize(const QString &path);
    void doneIndexing(int indexingTime);

protected slots:
    void saveSettings();
    void changeIndex();
    void newIndex();
    void startSearching();
    void showStatistic();
    void resultsCount();
    bool openDB();
    void setResultParPage(int count){m_resultParPage = count;}
    void tabCountChange(int count);
    void closeTab(int index);
    QString buildFilePath(QString bkid, int archive);


protected:
    IndexInfo *m_currentIndex;
    QSqlDatabase m_bookDB;
    QSqlQuery *m_bookQuery;
    QString m_titleName;
    QString m_bookTableName;
    QString m_searchQuery;
    QString m_highLightRE;
    QHash<QString, IndexInfo*> m_indexInfoMap;

    QProgressBar *m_fetechProgressBar;
    QLabel *m_searchTimeLabel;
    QLabel *m_searchResultsLabel;

    int m_resultParPage;
    int m_searchCount;
    bool m_dbIsOpen;
    bool m_haveMainTable;
    Ui::MainWindow *ui;

private slots:
    void displayResultsOptions();
    void on_lineQueryMust_returnPressed();
    void on_lineQueryShould_returnPressed();
    void on_lineQueryShouldNot_returnPressed();
};

#endif // MAINWINDOW_H
