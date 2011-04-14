#include "shamelabooksreader.h"
#include "common.h"
#include <qvariant.h>

ShamelaBooksReader::ShamelaBooksReader(QObject *parent) : QObject(parent)
{
    m_query = NULL;
    m_highLightAll = false;
}

ShamelaBooksReader::~ShamelaBooksReader()
{
    close();
}

void ShamelaBooksReader::setResult(ShamelaResult *result)
{
    if(result)
        m_shamelaResult = result;
}

void ShamelaBooksReader::setBookInfo(BookInfo *bookInfo)
{
    m_bookInfo = bookInfo;
}

bool ShamelaBooksReader::open()
{
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1").arg(m_bookInfo->path());
    QString connName = QString("shamelaBook_%1").arg(m_bookInfo->id());

    if(QSqlDatabase::contains(connName)) {
        m_bookDB = QSqlDatabase::database(connName);
    } else {
        m_bookDB = QSqlDatabase::addDatabase("QODBC", connName);
        m_bookDB.setDatabaseName(mdbpath);
    }
    if (!m_bookDB.open()) {
        DB_OPEN_ERROR(m_bookInfo->path());
        return false;
    }

    m_query = new QSqlQuery(m_bookDB);
    m_connections.append(connName);

    m_shoorts = m_booksDb->getBookShoorts(m_bookInfo->id());

    return true;
}

QString ShamelaBooksReader::homePage()
{
    return page(m_shamelaResult->pageID());
}

QString ShamelaBooksReader::nextPage()
{
    return page(++m_currentID);
}

QString ShamelaBooksReader::prevPage()
{
    return page(--m_currentID);
}

QString ShamelaBooksReader::page(int id)
{
    QString text;
    m_query->exec(QString("SELECT TOP 1 id, nass, page, part FROM %1 "
                             "WHERE id <= %2 ORDER BY id DESC")
                     .arg(m_bookInfo->mainTable())
                     .arg(id));

    if(m_query->first()) {
        text = m_query->value(1).toString();

        m_currentID = m_query->value(0).toInt();
        m_currentPage = m_query->value(2).toInt();
        m_currentPart = m_query->value(3).toInt();
    } else {
        qWarning("No page at: %d", id);
        return "Error occured!";
    }

    for(int i=0; i < m_shoorts.count(); i++)
        text.replace(m_shoorts.at(i).first, i ? '\n' + m_shoorts.at(i).second : m_shoorts.at(i).second);

    text.replace(QRegExp("[\\r\\n]"),"<br/>");

    clearShorts(text);

    if(m_highLightAll || (m_shamelaResult->pageID() == m_currentID))
        text = m_textHighlighter.hiText(text);

    return text;
}

QString ShamelaBooksReader::bookName()
{
    return m_bookInfo->name();
}

int ShamelaBooksReader::currentPage()
{
    return m_currentPage;
}

int ShamelaBooksReader::currentPart()
{
    return m_currentPart;
}

void ShamelaBooksReader::close()
{
    if(m_query != NULL) {
        delete m_query;
        m_query = NULL;
    }

    if(m_bookDB.isOpen())
        m_bookDB.close();

    m_textHighlighter.clear();
    m_shoorts.clear();
}

void ShamelaBooksReader::setStringTohighlight(QString str)
{
    m_textHighlighter.setStringToHighlight(str);
}

QStringList ShamelaBooksReader::connections()
{
    return m_connections;
}

void ShamelaBooksReader::setHighLightAll(bool hl)
{
    m_highLightAll = hl;
}

void ShamelaBooksReader::setBooksDB(BooksDB *booksdb)
{
    m_booksDb = booksdb;
}
