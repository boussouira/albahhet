#ifndef INDEXINGDIALG_H
#define INDEXINGDIALG_H

#include <QDialog>
#include "mainwindow.h"

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

private:
    Ui::IndexingDialg *ui;

private slots:
    void on_pushButton_clicked();
};

#endif // INDEXINGDIALG_H
