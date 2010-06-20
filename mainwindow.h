#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextBrowser>
#include <QFileDialog>
#include <QSettings>
#include <QSpinBox>
#include <QStandardItemModel>
#include "common.h"
#include "arabicanalyzer.h"
#include "indexingdialg.h"
#include "indexthread.h"

struct result{
    QList<int> ids;
    QList<int> bookid;
    QList<float_t> scoring;
    int page;
    int pageCount;
};

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    QString hiText(const QString &text, const QString &strToHi);
    QStringList buildRegExp(const QString &str);
    QString abbreviate(QString str, int size);
    QString getIndexSize();
    QString getBookSize();
    void writeLog(int indexingTime);
    QString getTitleId(int pageID);
    QString getBookName(int bookID);

public slots:
    void startIndexing();
    void startSearching();
    void showStatistic();
    void indexDocs(IndexWriter* writer);
    Document* FileDocument(const QString &id, const QString &ayaText);
    QString cleanString(QString str);
    void resultsCount();
    void displayResults(/*result &pResult*/);
    void setPageCount(int current, int count);
    void buttonStat(int currentPage, int pageCount);
    QStringList makeVLabels(int start, int end);
    bool openDB();
    void setResultParPage(int count){m_resultParPage = count;}
    QString buildFilePath(QString bkid);

protected:
    QSqlDatabase m_bookDB;
    QSqlQuery *m_bookQuery;
    QString m_quranDBPath;
    QString m_titleName;
    QString m_bookTableName;
    QString m_bookPath;
    QString m_bookName;
    QString m_searchQuery;
    QString m_highLightRE;
    QStandardItemModel *m_resultModel;
    QList<QString> m_colors;
    result m_resultStruct;
    int m_resultCount;
    int m_resultParPage;
    bool m_dbIsOpen;
    bool m_haveMainTable;
    Ui::MainWindow *ui;

private slots:
    void on_pushButton_2_clicked();
    void resultLinkClicked(const QUrl &url);
    void on_pushButton_clicked();
    void on_lineQuery_returnPressed();
    void on_pushGoLast_clicked();
    void on_pushGoFirst_clicked();
    void on_pushGoPrev_clicked();
    void on_pushGoNext_clicked();
};

#endif // MAINWINDOW_H
