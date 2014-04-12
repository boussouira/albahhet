#ifndef SHAMELABOOKSREADER_H
#define SHAMELABOOKSREADER_H

#include <qobject.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlerror.h>

#include "booksdb.h"
#include "bookinfo.h"
#include "shamelaresult.h"
#include "qthighlighter.h"
#include "qthighlighter.h"

class ShamelaBooksReader : public QObject
{
    Q_OBJECT
public:
    ShamelaBooksReader(QObject *parent=0);
    ~ShamelaBooksReader();
    void setResult(ShamelaResult *result);
    void setBooksDB(BooksDB *booksdb);
    void setBookInfo(BookInfo *bookInfo);
    void setStringTohighlight(QString str);
    void setHighLightAll(bool hl);
    bool open();
    void close();
    QStringList connections();

public slots:
    QString homePage();
    QString nextPage();
    QString prevPage();
    QString page(int id);
    QString bookName();
    int currentPage();
    int currentPart();

    void openInShamela();
    void openInViewer();

protected:
    QSqlDatabase m_bookDB;
    QtHighLighter m_textHighlighter;
    QSqlQuery *m_query;
    BooksDB *m_booksDb;
    BookInfo *m_bookInfo;
    ShamelaResult *m_shamelaResult;
    QStringList m_connections;
    QList<QPair<QString, QString> > m_shoorts;
    int m_currentPage;
    int m_currentPart;
    int m_currentID;
    bool m_highLightAll;
};

#endif // SHAMELABOOKSREADER_H
