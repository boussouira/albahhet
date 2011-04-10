#ifndef INDEXINGDIALG_H
#define INDEXINGDIALG_H

#include <QDialog>
#include <QTime>
#include <QSettings>
#include "booksdb.h"
#include "shamelaindexer.h"
#include "indexinfo.h"
#include "bookprogresswidget.h"
#include "indexesmanager.h"

namespace Ui {
    class IndexingDialg;
}

class QMessageBox;

class IndexingDialg : public QDialog {
    Q_OBJECT
public:
    IndexingDialg(QWidget *parent = 0);
    ~IndexingDialg();
    void setIndexesManager(IndexesManager *manager);
    void saveIndexInfo(int indexingTime=0, int opimizingTime=0);
    void checkIndex();

protected:
    void showBooks();
    void shutDown();

public slots:
    void addBook(const QString &name);
    void doneIndexing();
    void indexingError();
    void shutDownUpdateTime(qreal);
    void nextStep();
    void stopIndexing();
    void startIndexing();

signals:
    void indexCreated();

protected:
    IndexesManager *m_indexesManager;
    IndexWriter* m_writer;
    BooksDB *m_bookDB;
    IndexInfo *m_indexInfo;
    QTime m_indexingTime;
    QMessageBox *m_shutDownMsgBox;
    int m_booksCount;
    int m_indexedBooks;
    int m_threadCount;
    int m_shutDownTime;
    bool m_shutDown;
    Ui::IndexingDialg *ui;

private slots:
    void on_buttonSelectIndexPath_clicked();
    void on_buttonSelectShamela_clicked();
    void on_pushCancel_clicked();
    void on_label_2_linkActivated(QString);
};

#endif // INDEXINGDIALG_H
