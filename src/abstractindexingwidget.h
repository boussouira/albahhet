#ifndef ABSTRACTINDEXINGWIDGET_H
#define ABSTRACTINDEXINGWIDGET_H

#include <QWidget>
#include <QTime>
#include <CLucene/SharedHeader.h>

CL_CLASS_DEF(index, IndexWriter)
class IndexesManager;
class BooksDB;
class IndexInfo;
class QMessageBox;
class QPushButton;

class AbstractIndexingWidget : public QWidget
{
    Q_OBJECT

public:
    AbstractIndexingWidget(QWidget *parent = 0);
    virtual QString indexTypeName()=0;
    void setIndexManager(IndexesManager *manager);
    void setButtons(QPushButton *nextButton, QPushButton *cancelButton);
    void shutDown();

public slots:
    virtual void nextStep()=0;
    void shutDownUpdateTime(qreal);

signals:
    void indexCreated();
    void done();

protected:
    CL_NS2(index, IndexWriter)* m_writer;
    IndexesManager *m_indexesManager;
    BooksDB *m_bookDB;
    IndexInfo *m_indexInfo;
    QTime m_indexingTime;
    QMessageBox *m_shutDownMsgBox;
    QPushButton *m_nextButton;
    QPushButton *m_cancelButton;
    int m_shutDownTime;
    bool m_shutDown;
};

#endif // ABSTRACTINDEXINGWIDGET_H
