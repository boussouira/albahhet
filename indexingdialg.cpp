#include "indexthread.h"
#include "indexingdialg.h"
#include "ui_indexingdialg.h"

IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);
    ui->progressBar->setVisible(false);
    ui->pushStopIndexing->setVisible(false);
    ui->pushClose->setVisible(false);
    showBooks();

    m_indexing = new IndexingThread();
    connect(m_indexing, SIGNAL(fileIndexed(QString)), this, SLOT(addBook(QString)));
    connect(m_indexing, SIGNAL(finished()), this, SLOT(doneIndexing()));
}

IndexingDialg::~IndexingDialg()
{
    delete ui;
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

void IndexingDialg::on_pushStartIndexing_clicked()
{
    ui->listWidget->clear();
    ui->progressBar->setVisible(true);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(m_booksCount);
    ui->progressBar->setValue(0);
    m_indexedBooks =0;

    ui->pushStartIndexing->setVisible(false);
    ui->groupBox->setVisible(false);
    ui->pushStopIndexing->setVisible(true);

    m_indexing->setOptions(ui->checkOptimizeIndex->isChecked(),
                           ui->checkRamSize->isChecked() ? ui->spinRamSize->value() : 0,
                           ui->checkMaxDoc->isChecked() ? ui->spinMaxDoc->value() : 0);

    m_indexing->start();
    indexingTime.start();
}

void IndexingDialg::addBook(const QString &name)
{
    ui->progressBar->setValue(++m_indexedBooks);
    ui->listWidget->insertItem(ui->listWidget->count(), tr("%1 - %2").arg(m_indexedBooks).arg(name));
    ui->listWidget->scrollToBottom();
}

void IndexingDialg::doneIndexing()
{
    int elpasedMsec = indexingTime.elapsed();
    int seconds = (int) ((elpasedMsec / 1000) % 60);
    int minutes = (int) ((elpasedMsec / 1000) / 60);

    ui->pushStopIndexing->setVisible(false);
    ui->pushClose->setVisible(true);
    ui->progressBar->setVisible(false);

    QMessageBox::information(this,
                             trUtf8("تمت الفهرسة بنجاح"),
                             trUtf8("تمت فهرسة %1 كتابا خلال %2 و %3")
                             .arg(m_indexedBooks)
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

void IndexingDialg::on_pushStopIndexing_clicked()
{
    int rep = QMessageBox::question(this,
                                    trUtf8("فهرسة المكتبة"),
                                    trUtf8("هل تريد ايقاف فهرسة المكتبة"),
                                    QMessageBox::Yes|QMessageBox::No);
    if(rep==QMessageBox::Yes){
        m_indexing->stop();
        ui->pushStopIndexing->setEnabled(false);
        ui->progressBar->setMaximum(0);
    }
}

void IndexingDialg::on_pushClose_clicked()
{
    done(0);
}
