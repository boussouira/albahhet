#ifndef INDEXINGDIALG_H
#define INDEXINGDIALG_H

#include <QDialog>
#include <QTime>
#include "common.h"

namespace Ui {
    class IndexingDialg;
}

class IndexingThread;

class IndexingDialg : public QDialog {
    Q_OBJECT
public:
    IndexingDialg(QWidget *parent = 0);
    ~IndexingDialg();

protected:
    void showBooks();
    QString formatMinutes(int minutes);
    QString formatSecnds(int seconds);

public slots:
    void addBook(const QString &name);
    void doneIndexing();

protected:
    IndexingThread *m_indexing;
    int m_booksCount;
    int m_indexedBooks;
    QTime indexingTime;
    Ui::IndexingDialg *ui;

private slots:
    void on_pushClose_clicked();
    void on_pushStopIndexing_clicked();
    void on_pushStartIndexing_clicked();
};

#endif // INDEXINGDIALG_H
