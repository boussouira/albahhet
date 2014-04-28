#ifndef SHAMELAINDEXERWIDGET_H
#define SHAMELAINDEXERWIDGET_H

#include <qwizard.h>
#include "indexwidgetbase.h"

class ShamelaIndexerWidget : public IndexWidgetBase {
    Q_OBJECT

public:
    enum Pages {
        Page_SelectShamelaPage,
        Page_SelectBooksPage,
        Page_BooksIndexingPage,
        Page_DoneIndexingPage,
    };

public:
    ShamelaIndexerWidget(QWidget *parent);

    QString id();
    QString title();
    QString description();

    QList<QWizardPage*> pages();

    IndexInfo *indexInfo() const;
    BooksDB *bookDB() const;
    int booksCount() const;
    int indexedBooksCount() const;
    int indexingTime() const;
    int optimizeIndexTime() const;

    void setBooksCount(int booksCount);
    void setIndexedBooksCount(int indexedBooksCount);
    void setIndexingTime(int indexingTime);
    void setOptimizeIndexTime(int optimizeIndexTime);

    void saveIndexInfo();

    bool cancel();

protected:
    IndexInfo *m_indexInfo;
    BooksDB *m_bookDB;
    QList<QWizardPage*> m_pages;
    int m_booksCount;
    int m_indexedBooksCount;
    int m_indexingTime;
    int m_optimizeIndexTime;
};


#endif // SHAMELAINDEXERWIDGET_H
