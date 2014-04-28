#ifndef INDEXWIDGETBASE_H
#define INDEXWIDGETBASE_H

#include <QWidget>
#include <QTime>

namespace lucene {
    namespace index {
        class IndexWriter;
    }
}

class IndexesManager;
class BooksDB;
class IndexInfo;
class QMessageBox;
class QPushButton;
class QWizardPage;

class IndexWidgetBase : public QWidget
{
    Q_OBJECT
public:
    IndexWidgetBase(QWidget *parent);

    virtual QString id()=0;
    virtual QString title()=0;
    virtual QString description()=0;

    virtual QList<QWizardPage*> pages()=0;

    virtual bool cancel() { return true; }

    void openIndexWriter(QString indexPath);
    void closeIndexWriter(bool close);

    void enableHibernateMode(bool enable);

    IndexesManager *indexesManager() const;
    lucene::index::IndexWriter *indexWriter() const;

    void setIndexesManager(IndexesManager *indexesManager);

signals:
    void indexCreated();

protected:
    IndexesManager *m_indexesManager;
    lucene::index::IndexWriter *m_writer;
};

#endif // INDEXWIDGETBASE_H