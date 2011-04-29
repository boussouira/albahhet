#include "indexingdialg.h"
#include "ui_indexingdialg.h"
#include "common.h"
#include "bookinfo.h"
#include "settingsdialog.h"
#include "settingschecker.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtimeline.h>
#include "shamelaindexerwidget.h"

IndexingDialg::IndexingDialg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IndexingDialg)
{
    ui->setupUi(this);

    connect(ui->pushNext, SIGNAL(clicked()), SLOT(showIndexingWidget()));
    connect(ui->pushCancel, SIGNAL(clicked()), SLOT(reject()));
}

IndexingDialg::~IndexingDialg()
{
    delete ui;
}

void IndexingDialg::setIndexesManager(IndexesManager *manager)
{
    m_indexesManager = manager;
}

void IndexingDialg::showIndexingWidget()
{
    if(ui->radioShamela->isChecked())
        m_indexWidget = new ShamelaIndexerWidget(this);
    else
        qFatal("Unknow index type");

    m_indexWidget->setIndexManager(m_indexesManager);
    m_indexWidget->setButtons(ui->pushNext, ui->pushCancel);

    ui->verticalLayout->insertWidget(0, m_indexWidget);
    ui->widgetSelectIndexType->hide();

    disconnect(ui->pushNext, SIGNAL(clicked()), this, SLOT(showIndexingWidget()));
    connect(ui->pushNext, SIGNAL(clicked()), m_indexWidget, SLOT(nextStep()));
    connect(m_indexWidget, SIGNAL(done()), this, SLOT(doneIndexing()));
    connect(m_indexWidget, SIGNAL(indexCreated()), SIGNAL(indexCreated()));
}

void IndexingDialg::doneIndexing()
{
    delete m_indexWidget;
    accept();
}
