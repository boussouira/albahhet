#include "indexingdialg.h"
#include "ui_indexingdialg.h"
#include <QStringListModel>

IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
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
    m_booksCount = 0;
    QSqlDatabase indexDB = QSqlDatabase::addDatabase("QSQLITE", "bookIndexDiaog");
    indexDB.setDatabaseName("book_index.db");
    if(!indexDB.open())
        qDebug("Error opning index db");
    QSqlQuery *inexQuery = new QSqlQuery(indexDB);
    QStringList booksList;

    inexQuery->exec("SELECT shamelaID, bookName, filePath FROM books");
    while(inexQuery->next()) {
        booksList.append(inexQuery->value(1).toString());
        m_booksCount++;
    }
    ui->listWidget->insertItems(0, booksList);
}

void IndexingDialg::on_pushButton_clicked()
{
    ui->listWidget->clear();
    IndexThread *indexing = new IndexThread();
    ui->progressBar->setVisible(true);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(m_booksCount);
    ui->progressBar->setValue(0);
    connect(indexing, SIGNAL(fileIndexed(QString)), this, SLOT(addBook(QString)));
    connect(indexing, SIGNAL(finished()), this, SLOT(doneIndexing()));

    indexing->start();
    indexingTime.start();
}

void IndexingDialg::addBook(const QString &name)
{
    int val = ui->progressBar->value();
    ui->progressBar->setValue(++val);
    ui->listWidget->insertItem(ui->listWidget->count(), tr("%1 - %2").arg(val).arg(name));
    ui->listWidget->scrollToBottom();
}

void IndexingDialg::doneIndexing()
{
    int elpasedMsec = indexingTime.elapsed();
    int seconds = (int) ((elpasedMsec / 1000) % 60);
    int minutes = (int) ((elpasedMsec / 1000) / 60);

    ui->progressBar->setVisible(false);
    QMessageBox::information(this,
                             trUtf8("تمت الفهرسة بنجاح"),
                             trUtf8("تمت فهرسة %1 كتابا خلال %2 و %3")
                             .arg(m_booksCount)
                             .arg(formatMinutes(minutes))
                             .arg(formatSecnds(seconds)));
}

QString IndexingDialg::formatMinutes(int minutes)
{
    if(minutes == 1)
        return trUtf8("دقيقة");
    else if(minutes == 2)
        return trUtf8("دقيقتين");
    else if(9 >= minutes && minutes > 2)
        return trUtf8("%1 دقائق").arg(minutes);
    else
        return trUtf8("%1 دقيقة").arg(minutes);
}

QString IndexingDialg::formatSecnds(int seconds)
{
    if(seconds == 1)
        return trUtf8("ثانية");
    else if(seconds == 2)
        return trUtf8("ثانيتين");
    else if(9 >= seconds && seconds > 2)
        return trUtf8("%1 ثوان").arg(seconds);
    else
        return trUtf8("%1 ثانية").arg(seconds);
}
