#ifndef CREATEINDEXDIALOG_H
#define CREATEINDEXDIALOG_H

#include <qwizard.h>

class IndexWidgetBase;
class IndexesManager;
class IndexSelectionPage;

class CreateIndexDialog : public QWizard
{
    Q_OBJECT
public:
    CreateIndexDialog(IndexesManager *indexesManager, QWidget *parent = 0);

    IndexesManager *indexesManager() const;
    void setIndexesManager(IndexesManager *indexesManager);

    IndexWidgetBase *getIndexWidgetById(QString wid);

    bool setupIndexWidget();

protected:
    void init();

public slots:
    void cancel();

signals:
    void indexCreated();

protected:
    IndexesManager *m_indexesManager;
    IndexSelectionPage *m_selectionPage;
    IndexWidgetBase *m_currentWidget;
    QList<IndexWidgetBase*> m_baseWidgets;
};

#endif // CREATEINDEXDIALOG_H
