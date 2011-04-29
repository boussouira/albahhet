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
#include "abstractindexingwidget.h"

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

public slots:
    void showIndexingWidget();
    void doneIndexing();

signals:
    void indexCreated();

protected:
    AbstractIndexingWidget *m_indexWidget;
    IndexesManager *m_indexesManager;
    Ui::IndexingDialg *ui;

};

#endif // INDEXINGDIALG_H
