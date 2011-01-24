#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qmainwindow.h>
#include <qsqldatabase.h>
#include <qhash.h>

namespace Ui {
    class MainWindow;
}

class IndexInfo;
class QSqlQuery;
class QProgressBar;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void loadIndexesList();
    void selectIndex(QString name);
    void selectIndex(QAction *action);
    void indexChanged();

protected:
    void closeEvent(QCloseEvent *e);
    QString getIndexSize();
    QString getBooksSize();
    qint64 getDirSize(const QString &path);
    void doneIndexing(int indexingTime);

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
    bool m_useMultiTab;
    Ui::MainWindow *ui;

private slots:
    void showSettingsDialog();
    void on_lineQueryMust_returnPressed();
    void on_lineQueryShould_returnPressed();
    void on_lineQueryShouldNot_returnPressed();
};

#endif // MAINWINDOW_H
