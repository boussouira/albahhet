#include "indexingdialg.h"
#include "ui_indexingdialg.h"
#include <QStringListModel>

IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);
    showBooks();
}

IndexingDialg::~IndexingDialg()
{
    delete ui;
}

void IndexingDialg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void IndexingDialg::showBooks()
{
    QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexDiaog");
    indexDB.setDatabaseName("book_index.db");
    if(!indexDB.open())
        qDebug("Error opning index db");
    QSqlQuery *inexQuery = new QSqlQuery(indexDB);
    QStringListModel *booksListModel = new QStringListModel(this);
    QStringList booksList;

    inexQuery->exec("SELECT * FROM books");
    while(inexQuery->next()) {
        booksList.append(inexQuery->value(1).toString());
    }
    booksListModel->setStringList(booksList);
    ui->listView->setModel(booksListModel);
}

void IndexingDialg::on_pushButton_clicked()
{

}
