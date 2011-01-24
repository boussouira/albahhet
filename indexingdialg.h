#ifndef INDEXINGDIALG_H
#define INDEXINGDIALG_H

#include <QDialog>
#include <QTime>
#include <QSettings>
#include "booksdb.h"
#include "indexthread.h"
#include "indexinfo.h"

namespace Ui {
    class IndexingDialg;
}

class IndexingDialg : public QDialog {
    Q_OBJECT
public:
    IndexingDialg(QWidget *parent = 0);
    ~IndexingDialg();
    static QString arPlural(int count, int word);
    void saveIndexInfo();
    void checkIndex();

protected:
    void showBooks();
    QString formatTime(int milsec);

public slots:
    void addBook(const QString &name);
    void doneIndexing();
    void indexingError();
    void setRamSize();

signals:
    void indexCreated();

protected:
    IndexWriter* m_writer;
    BooksDB *m_bookDB;
    IndexInfo *m_indexInfo;
    QTime indexingTime;
    int m_booksCount;
    int m_indexedBooks;
    int m_threadCount;
    bool m_stopIndexing;
    Ui::IndexingDialg *ui;

private slots:
    void on_buttonSelectIndexPath_clicked();
    void on_buttonSelectShamela_clicked();
    void nextStep();
    void on_pushCancel_clicked();
    void stopIndexing();
    void startIndexing();
};

#endif // INDEXINGDIALG_H
