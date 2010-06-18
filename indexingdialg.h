#ifndef INDEXINGDIALG_H
#define INDEXINGDIALG_H

#include <QDialog>
#include <QTime>
#include "mainwindow.h"
#include "indexthread.h"

namespace Ui {
    class IndexingDialg;
}

class IndexingDialg : public QDialog {
    Q_OBJECT
public:
    IndexingDialg(QWidget *parent = 0);
    ~IndexingDialg();

protected:
    void changeEvent(QEvent *e);
    void showBooks();
    QString formatMinutes(int minutes);
    QString formatSecnds(int seconds);

public slots:
    void addBook(const QString &name);
    void doneIndexing();

private:
    int m_booksCount;
    QTime indexingTime;
    Ui::IndexingDialg *ui;

private slots:
    void on_pushButton_clicked();
};

#endif // INDEXINGDIALG_H
