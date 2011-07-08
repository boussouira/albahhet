#include "quranindexer.h"
#include "common.h"
#include "booksdb.h"
#include "indexinfo.h"
#include "bookinfo.h"
#include <qvariant.h>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qmessagebox.h>

QuranIndexer::QuranIndexer()
{
    m_prevSora = -1;
}

void QuranIndexer::run()
{
    startIndexing();
}

void QuranIndexer::startIndexing()
{
    try {
        indexQuran();
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "CLucene Error when Indexing",
                             tr("Error code: %1\n%2").arg(err.number()).arg(err.what()));
        emit indexingError();
        terminate();
    }
    catch(std::exception &err){
        QMessageBox::warning(0, "Error when Indexing",
                             tr("exception: %1").arg(err.what()));
        emit indexingError();
        terminate();
    }
    catch(...){
        QMessageBox::warning(0, "Unkonw error when Indexing",
                             tr("Unknow error"));
        emit indexingError();
        terminate();
    }
}

void QuranIndexer::indexQuran()
{
    QSqlDatabase mdbDB;
    QString connName("_quran_1");


    mdbDB = QSqlDatabase::addDatabase("QODBC", connName);
    mdbDB.setDatabaseName(QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                          .arg(m_indexInfo->shamelaSpecialDbPath()));

    if(!mdbDB.isOpen()) {
        if (!mdbDB.open()) {
            DB_OPEN_ERROR(m_indexInfo->shamelaSpecialDbPath());
            return;
        }
    }

    int prevSora = -1;

    QSqlQuery shaQuery(mdbDB);
    if(!shaQuery.exec("SELECT id, sora, nass FROM Qr ORDER BY id "))
        SQL_ERROR(shaQuery.lastError().text());

    Document doc;

    while(shaQuery.next())
    {
        doc.add( *_CLNEW Field(_T("id"), QSTRING_TO_TCHAR(shaQuery.value(0).toString()),   Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        doc.add( *_CLNEW Field(_T("sora"), QSTRING_TO_TCHAR(shaQuery.value(1).toString()), Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        doc.add( *_CLNEW Field(_T("text"), QSTRING_TO_TCHAR(shaQuery.value(2).toString()), Field::STORE_NO | Field::INDEX_TOKENIZED));

        m_writer->addDocument(&doc);

        doc.clear();
        if(prevSora != shaQuery.value(1).toInt()) {
            prevSora = shaQuery.value(1).toInt();
            emit currentSoraName(m_bookDB->getSoraName(prevSora));
        }
    }
}
