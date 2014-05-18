#ifndef SHAMELAINDEXERPAGES_H
#define SHAMELAINDEXERPAGES_H

#include <QWizardPage>
#include <QTime>

class ShamelaIndexerWidget;
class IndexOptimizer;
class BookProgressWidget;
class QListWidget;
class QProgressBar;
class QVBoxLayout;
class QCheckBox;
class QLabel;

class ShamelaPageBase : public QWizardPage
{
    Q_OBJECT

public:
    ShamelaPageBase(ShamelaIndexerWidget *parent);

    virtual bool cancel();
protected:
    ShamelaIndexerWidget *m_parent;
};


class SelectShamelaPage : public ShamelaPageBase
{
    Q_OBJECT

public:
    SelectShamelaPage(ShamelaIndexerWidget *parent);

    bool validatePage();

protected slots:
    void openSettings();
};

class SelectBooksPage : public ShamelaPageBase
{
public:
    SelectBooksPage(ShamelaIndexerWidget *parent);

    void initializePage();
    bool validatePage();
    int nextId() const;

    void checkIndex();
    void showBooks();

protected:
    QListWidget *m_listWidget;
    bool m_useExistingIndex;
};


class BooksIndexingPage : public ShamelaPageBase
{
    Q_OBJECT
public:
    BooksIndexingPage(ShamelaIndexerWidget *parent);

    void initializePage();
    bool isComplete () const;
    bool cancel();

    void startIndexing();

public slots:
    void addBook(const QString &);
    void indexThreadFinished();
    void indexThreadError();
    void indexingDone();

protected:
    QList<BookProgressWidget*> m_bookProgressList;
    IndexOptimizer *m_optimizer;
    QVBoxLayout *m_bookNameBox;
    QProgressBar *m_progressBar;
    QCheckBox *m_checkOptimizeIndex;
    QTime m_indexingTime;
    int m_indexedBooks;
    int m_threadCount;
    bool m_done;
};

class DoneIndexingPage : public ShamelaPageBase
{
public:
    DoneIndexingPage(ShamelaIndexerWidget *parent);

    void initializePage();

protected:
    QLabel *m_label;
};

#endif // SHAMELAINDEXERPAGES_H
