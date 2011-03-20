#ifndef SHAMELABOOKSREADER_H
#define SHAMELABOOKSREADER_H

#include <qobject.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlerror.h>

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
    void setBookInfo(BookInfo *bookInfo);
    void setStringTohighlight(QString str);
    bool open();
    void close();

public slots:
    QString homePage();
    QString nextPage();
    QString prevPage();
    QString page(int id, bool hihgLight=false);
    QString bookName();
    int currentPage();
    int currentPart();

protected:
    QSqlDatabase m_bookDB;
    QtHighLighter m_textHighlighter;
    QSqlQuery *m_query;
    BookInfo *m_bookInfo;
    ShamelaResult *m_shamelaResult;
    int m_currentPage;
    int m_currentPart;
    int m_currentID;
};

#endif // SHAMELABOOKSREADER_H
