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

#include "common.h"
#include "arabicanalyzer.h"
#include "indexingdialg.h"
#include "indexthread.h"
#include "shamelasearcher.h"

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
    QString hiText(const QString &text, const QString &strToHi);
    QStringList buildRegExp(const QString &str);
    QString abbreviate(QString str, int size);
    QString getIndexSize();
    QString getBooksSize();
    qint64 getDirSize(const QString &path);
    void doneIndexing(int indexingTime);
    QString getTitleId(const QSqlDatabase &db, int pageID, int archive, int bookID);
    QString getBookName(int bookID);

public slots:
    void searchStarted();
    void searchFinnished();
    void fetechStarted();
    void fetechFinnished();
    void gotResult(ShamelaResult *result);

    void changeIndex();
    void newIndex();
    void startSearching();
    void showStatistic();
    QString cleanString(QString str);
    void resultsCount();
    void displayResults();
    void setPageCount(int current, int count);
    void buttonStat(int currentPage, int pageCount);
    bool openDB();
    void setResultParPage(int count){m_resultParPage = count;}
    QString buildFilePath(QString bkid, int archive);
    void populateJavaScriptWindowObject();

    QString getPage(QString href);
    QString formNextUrl(QString href);
    QString formPrevUrl(QString href);
    void updateNavgitionLinks(QString href);

protected:
    ShamelaSearcher *m_searcher;
    IndexInfo *m_currentIndex;
    QSqlDatabase m_bookDB;
    QSqlQuery *m_bookQuery;
    QString m_titleName;
    QString m_bookTableName;
    QString m_searchQuery;
    QString m_highLightRE;
    QHash<QString, IndexInfo*> m_indexInfoMap;
    QList<QString> m_colors;
    QProgressBar *m_fetechProgressBar;
    QLabel *m_searchTimeLabel;
    QLabel *m_searchResultsLabel;
    int m_resultCount;
    int m_resultParPage;
    int m_currentShownId;
    bool m_dbIsOpen;
    bool m_haveMainTable;
    Ui::MainWindow *ui;

private slots:
    void displayResultsOptions();
    void resultLinkClicked(const QUrl &url);
    void on_lineQuery_returnPressed();
    void on_pushGoLast_clicked();
    void on_pushGoFirst_clicked();
    void on_pushGoPrev_clicked();
    void on_pushGoNext_clicked();
};

#endif // MAINWINDOW_H
