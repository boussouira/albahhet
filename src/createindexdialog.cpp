#include "createindexdialog.h"
#include "shamelaindexerwidget.h"
#include "indexesmanager.h"

#include <qlabel.h>
#include <QVBoxLayout>
#include <qpushbutton.h>
#include <qradiobutton.h>

enum {
    IndexWidgetId = 10
};

class IndexSelectionPage : public QWizardPage
{

public:
    IndexSelectionPage(CreateIndexDialog *parent) :
        QWizardPage(parent), m_parent(parent)
    {
        setTitle(tr("انشاء فهرس"));
        setSubTitle(tr("من فضلك اختر نوع الفهرس الذي تريد انشاءه"));

        QVBoxLayout *layout = new QVBoxLayout(this);

        QLabel *label = new QLabel(this);
        layout->addWidget(label);

    }

    void initializePage()
    {
        foreach (IndexWidgetBase *w, m_baseWidgets) {
            QRadioButton *radio = new QRadioButton(w->title(), this);
            radio->setToolTip(w->description());
            radio->setObjectName(w->id());

            if(m_radioButtons.isEmpty())
                radio->setChecked(true);

            m_radioButtons.append(radio);

            layout()->addWidget(radio);
        }

        //layout()->addStretch();
    }

    bool validatePage()
    {
        if(m_parent->setupIndexWidget()) {
           QTimer::singleShot(0, m_parent, SLOT(restart()));
        }

        return false;
    }

    CreateIndexDialog *m_parent;
    QList<IndexWidgetBase*> m_baseWidgets;
    QList<QRadioButton*> m_radioButtons;

};

CreateIndexDialog::CreateIndexDialog(IndexesManager *indexesManager, QWidget *parent) :
    QWizard(parent), m_indexesManager(indexesManager), m_currentWidget(0)
{
    setWindowTitle(tr("انشاء فهرس"));

    setOption(QWizard::DisabledBackButtonOnLastPage);
    setOption(QWizard::HaveNextButtonOnLastPage);

    button(QWizard::CancelButton)->disconnect();

    init();

    connect(button(QWizard::CancelButton), SIGNAL(clicked()),
            SLOT(cancel()));
}
IndexesManager *CreateIndexDialog::indexesManager() const
{
    return m_indexesManager;
}

void CreateIndexDialog::setIndexesManager(IndexesManager *indexesManager)
{
    m_indexesManager = indexesManager;
}

void CreateIndexDialog::init()
{
    m_baseWidgets.append(new ShamelaIndexerWidget(this));

    foreach(IndexWidgetBase *p, m_baseWidgets) {
        p->setIndexesManager(m_indexesManager);
    }

    m_selectionPage = new IndexSelectionPage(this);
    m_selectionPage->m_baseWidgets = m_baseWidgets;

    addPage(m_selectionPage);
    addPage(new QWizardPage(this));

}

//int CreateIndexDialog::nextId() const
//{
//    setupIndexWidget();
//    if(0)
//        return 0;
//    else
//        return 1;
//}

IndexWidgetBase *CreateIndexDialog::getIndexWidgetById(QString wid)
{
    foreach(IndexWidgetBase *w, m_baseWidgets) {
        if(w->id() == wid)
            return w;
    }

    return 0;
}

bool CreateIndexDialog::setupIndexWidget()
{
    QList<QRadioButton*> radioButtons = m_selectionPage->m_radioButtons;
    foreach(QRadioButton *radio, radioButtons) {
        if(radio->isChecked()) {
            m_currentWidget = getIndexWidgetById(radio->objectName());
            if(!m_currentWidget)
                continue;

            removePage(0);
            removePage(1);

            foreach(QWizardPage *p, m_currentWidget->pages())
                addPage(p);

            connect(m_currentWidget, SIGNAL(indexCreated()), SIGNAL(indexCreated()));
            return true;
        }
    }

    m_currentWidget = 0;
    return false;
}

void CreateIndexDialog::cancel()
{
    if(m_currentWidget && m_currentWidget->cancel())
        reject();
    else if(!m_currentWidget)
        reject();
}

