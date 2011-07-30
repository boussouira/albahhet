#ifndef QURANSEARCHWIDGET_H
#define QURANSEARCHWIDGET_H

#include "abstractsearchwidget.h"
#include "quransearcher.h"
#include "quranresult.h"

namespace Ui {
    class QuranSearchWidget;
}
class WebView;

class QuranSearchWidget : public AbstractSearchWidget
{
    Q_OBJECT

public:
    QuranSearchWidget(QWidget *parent = 0);
    ~QuranSearchWidget();
    void setIndexInfo(IndexInfo *info);
    void setBooksDb(BooksDB *db);
    void setTabWidget(TabWidget *tabWidget);
    void indexChanged();
    void saveSettings();
    void loadSettings();

public slots:
    void showNavigationButton(bool show);

protected slots:
    void gotResult(QuranResult *result);
    void searchStarted();
    void searchFinnished();
    void fetechStarted();
    void fetechFinnished();
    void gotException(QString what, int id);
    void populateJavaScriptWindowObject();
    void updateNavigation(int currentPage, int pageCount);
    void buttonStat(int currentPage, int pageCount);

protected:
    IndexInfo *m_indexInfo;
    BooksDB *m_booksDB;
    TabWidget *m_tabWidget;
    WebView *m_webView;
    QuranSearcher *m_searcher;
    int m_resultParPage;
    Ui::QuranSearchWidget *ui;

private slots:
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();
    void on_buttonGoFirst_clicked();
    void on_buttonGoLast_clicked();
    void on_buttonGoPrev_clicked();
    void on_buttonGoNext_clicked();
};

#endif // QURANSEARCHWIDGET_H
