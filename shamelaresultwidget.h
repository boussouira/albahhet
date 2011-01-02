#ifndef SHAMELARESULTWIDGET_H
#define SHAMELARESULTWIDGET_H

#include "shamelasearcher.h"
#include "common.h"
#include "arabicanalyzer.h"
#include "indexingdialg.h"
#include "indexthread.h"
#include "shamelasearcher.h"
#include "shamelaresultwidget.h"

#include <qwidget.h>
#include <qtextbrowser.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qstandarditemmodel.h>
#include <qaction.h>
#include <qhash.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qwebframe.h>

namespace Ui {
    class ShamelaResultWidget;
}

class ShamelaResultWidget : public QWidget
{
    Q_OBJECT

public:
    ShamelaResultWidget(QWidget *parent = 0);
    ~ShamelaResultWidget();
    void setShamelaSearch(ShamelaSearcher *s);
    void setIndexInfo(IndexInfo *info) { m_indexInfo = info; }
    void doSearch();

public slots:
    QString getPage(QString href);
    void updateNavgitionLinks(QString href);

protected slots:
    void searchStarted();
    void searchFinnished();
    void fetechStarted();
    void fetechFinnished();
    void gotResult(ShamelaResult *result);
    void gotException(QString what, int id);
    void populateJavaScriptWindowObject();
    void resultLinkClicked(const QUrl &url);

protected:
    QString buildFilePath(QString bkid, int archive);
    QString hiText(const QString &text, const QString &strToHi);
    QStringList buildRegExp(const QString &str);
    QString abbreviate(QString str, int size);
    QString cleanString(QString str);
    QString getTitleId(const QSqlDatabase &db, int pageID, int archive, int bookID);
    QString getBookName(int bookID);
    QString formNextUrl(QString href);
    QString formPrevUrl(QString href);
    void setPageCount(int current, int count);
    void buttonStat(int currentPage, int pageCount);

protected:
    ShamelaSearcher *m_searcher;
    IndexInfo *m_indexInfo;
    QList<QString> m_colors;
    int m_currentShownId;
    Ui::ShamelaResultWidget *ui;

private slots:
    void on_buttonGoFirst_clicked();
    void on_buttonGoLast_clicked();
    void on_buttonGoPrev_clicked();
    void on_buttonGoNext_clicked();
};

#endif // SHAMELARESULTWIDGET_H
