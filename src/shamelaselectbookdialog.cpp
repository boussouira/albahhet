#include "shamelaselectbookdialog.h"
#include "ui_shamelaselectbookdialog.h"
#include "bookinfo.h"

ShamelaSelectBookDialog::ShamelaSelectBookDialog(BooksDB *db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShamelaSelectBookDialog)
{
    ui->setupUi(this);

    m_booksDB = db;
    m_filter = new ShamelaFilterProxyModel(this);
    selectedBookID = 0;
    selectedBookVersion = 0;

    ui->treeView->setModel(m_filter);

    loadbooksList();

    connect(ui->lineSearch, SIGNAL(textChanged(QString)), m_filter, SLOT(setFilterRegExp(QString)));
    connect(ui->pushSelect, SIGNAL(clicked()), SLOT(selectBook()));
    connect(ui->pushCancel, SIGNAL(clicked()), SLOT(cancel()));
}

ShamelaSelectBookDialog::~ShamelaSelectBookDialog()
{
    delete ui;
}

void ShamelaSelectBookDialog::loadbooksList()
{
    m_model = m_booksDB->getSimpleBooksListModel();

    m_filter->setSourceModel(m_model);
}

void ShamelaSelectBookDialog::selectBook()
{
    if(ui->treeView->selectionModel()->selectedIndexes().isEmpty())
        return;

    QModelIndex index = ui->treeView->selectionModel()->selectedIndexes().first();
    BookInfo *info = m_booksDB->getBookInfo(index.data(BooksDB::idRole).toInt());

    selectedBookID = info->id();
    selectedBookVersion = info->bookVersion();
    selectedBookName = info->name();

    accept();
}

void ShamelaSelectBookDialog::cancel()
{
}
