#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "common.h"
#include "indexingdialg.h"
#include "shamelasearchwidget.h"
#include "settingsdialog.h"
#include "indexesdialog.h"
#include "logdialog.h"
#include "tabwidget.h"

#include <qtextbrowser.h>
#include <qfile.h>
#include <qdir.h>
#include <qsettings.h>
#include <qlabel.h>
#include <qtreewidget.h>
#include <qevent.h>
#include <qmessagebox.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME);

    m_indexesManager = new IndexesManager;

    m_tabWidget = new TabWidget(this);
    m_searchWidget = new ShamelaSearchWidget(m_tabWidget);
    m_searchWidget->setTabWidget(m_tabWidget);

    m_tabWidget->addTab(m_searchWidget,
                        QIcon(":/bin/data/images/find.png"),
                        trUtf8("بحث"));
    setCentralWidget(m_tabWidget);

    m_currentIndex = 0;
    m_booksDB = new BooksDB();

    m_logDialog = new LogDialog(this);
    m_logDialog->hide();
    hideHelpButton(m_logDialog);

    loadSettings();

    connect(ui->actionNewIndex, SIGNAL(triggered()), SLOT(newIndex()));
    connect(ui->actionIndexInfo, SIGNAL(triggered()), SLOT(showStatistic()));
    connect(ui->actionSearchSettings, SIGNAL(triggered()), SLOT(showSettingsDialog()));
    connect(ui->actionEditIndexes, SIGNAL(triggered()), SLOT(editIndexes()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(aboutApp()));
    connect(ui->actionLogDialog, SIGNAL(triggered()), SLOT(showLogDialog()));
}

MainWindow::~MainWindow()
{
    DELETE_DB(m_booksDB);
    delete m_logDialog;
    delete m_tabWidget;
    delete m_indexesManager;
    delete ui;
}

void MainWindow::saveSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("maximized", isMaximized());
    settings.endGroup();

    if(m_currentIndex && m_currentIndex->id() != -1)
        settings.setValue("current_index", m_currentIndex->id());

    m_searchWidget->saveSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings;
    m_showNewIndexMsg = settings.value("showNewIndexMsg", true).toBool();

    settings.beginGroup("MainWindow");
    resize(settings.value("size", size()).toSize());
    move(settings.value("pos", pos()).toPoint());

    if(settings.value("maximized", true).toBool())
        showMaximized();

    settings.endGroup();
}

void MainWindow::loadIndexesList()
{
    qDebug("Loading Indexes List...");
    QSettings settings;
    QList<IndexInfo *> indexesList =  m_indexesManager->list();
    int current;
    bool haveIndexes = !indexesList.isEmpty();

    if(haveIndexes) {
        current = settings.value("current_index", -1).toInt();
        if(current <= 0 || !m_indexesManager->idExists(current)) {
            current = indexesList.first()->id();
            qDebug("Select first id: %d", current);
        }

        for(int i=0; i<indexesList.count(); i++) {
            IndexInfo *info = indexesList.at(i);

            QAction *action = new QAction(info->name(), ui->menuIndexesList);
            action->setData(info->id());
            connect(action, SIGNAL(triggered()), SLOT(changeIndex()));

            ui->menuIndexesList->addAction(action);
        }
    } else {
        qDebug("No index found");
    }

    m_tabWidget->setEnabled(haveIndexes);
    ui->actionIndexInfo->setEnabled(haveIndexes);
    ui->actionEditIndexes->setEnabled(haveIndexes);
    ui->menuIndexesList->setEnabled(haveIndexes);

    if(haveIndexes)
        selectIndex(current);
}

void MainWindow::selectIndex(int id)
{
    foreach(QAction *action, ui->menuIndexesList->actions()) {
        if(action->data().toInt() == id) {
            selectIndex(action);
            break;
        }
    }
}

void MainWindow::selectIndex(QAction *action)
{
    qDebug("Select index: %d", action->data().toInt());
    m_currentIndex = m_indexesManager->indexInfo(action->data().toInt());

    if(m_currentIndex) {
        action->setCheckable(true);
        action->setChecked(true);

        bool haveIndex;

        try {
            indexChanged();
            haveIndex = true;
        } catch (QString &str) {
            QMessageBox::critical(this, trUtf8("تحميل فهرس"),
                                  trUtf8("حدث خطأ عند تحميل الفهرس:"
                                         "\n"
                                         "%1").arg(str));
            haveIndex = false;
        }

        m_tabWidget->setEnabled(haveIndex);
        ui->actionIndexInfo->setEnabled(haveIndex);
    } else {
        qFatal("No index with id %d", action->data().toInt());
    }
}

void MainWindow::changeIndex()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action) {
        if(action->isCheckable()) {
            action->setChecked(true);
            return;
        }

        foreach(QAction *a, ui->menuIndexesList->actions()) {
            a->setChecked(false);
            a->setCheckable(false);
        }

        selectIndex(action);
    }
}

void MainWindow::indexChanged()
{
    QSettings settings;

    if(m_currentIndex && m_currentIndex->id() != -1)
        settings.setValue("current_index", m_currentIndex->id());

    setWindowTitle(QString("%1 - %2").arg(APP_NAME).arg(m_currentIndex->name()));

    DELETE_DB(m_booksDB);

    m_booksDB = new BooksDB();
    m_booksDB->setIndexInfo(m_currentIndex);

    m_searchWidget->setIndexInfo(m_currentIndex);
    m_searchWidget->setBooksDb(m_booksDB);
    m_searchWidget->indexChanged();
}

void MainWindow::haveIndexesCheck()
{
    // Check if we have any index
    if(ui->menuIndexesList->actions().isEmpty() && m_showNewIndexMsg) {
        int rep = QMessageBox::question(this,
                                        trUtf8("انشاء فهرس"),
                                        trUtf8("لم يتم العثور على اي فهرس." "\n" "هل تريد انشاء فهرس جديد؟"),
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

        if(rep == QMessageBox::Yes)
            newIndex();
        else
            QMessageBox::information(this,
                                     trUtf8("انشاء فهرس"),
                                     trUtf8("يمكن انشاء فهرس جديد في اي وقت من خلال قائمة "
                                            "<strong>" "فهرس" "</strong>"
                                            " ثم "
                                            "<strong>" "انشاء فهرس جديد..." "</strong>"));

    }
}

void MainWindow::updateIndexesMenu()
{
    qDebug("update Indexes Menu");
    QList<QAction*> list = ui->menuIndexesList->actions();

    if(!list.isEmpty())
        qDeleteAll(list);

    m_indexesManager->clear();

    loadIndexesList();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void MainWindow::newIndex()
{
    IndexingDialg indexDialog(this);
    indexDialog.setIndexesManager(m_indexesManager);
    connect(&indexDialog, SIGNAL(indexCreated()), SLOT(updateIndexesMenu()));

    indexDialog.exec();
}

void MainWindow::editIndexes()
{
    IndexesDialog dialog(m_indexesManager, this);
    connect(&dialog, SIGNAL(indexesChanged()), SLOT(updateIndexesMenu()));

    dialog.exec();
}

void MainWindow::showStatistic()
{
    try {
        IndexReader* r = IndexReader::open(qPrintable(m_currentIndex->path()));
        //int64_t ver = r->getCurrentVersion(qPrintable(m_currentIndex->path()));

        QTreeWidget *treeWidget = new QTreeWidget;
        treeWidget->setColumnCount(2);
        treeWidget->setHeaderHidden(true);
        treeWidget->setRootIsDecorated(false);

        QList<QTreeWidgetItem *> itemList;
        IndexingInfo *info = m_currentIndex->indexingInfo();

        ADD_QTREEWIDGET_ITEM("اسم الفهرس", m_currentIndex->name());

        if(info)
            ADD_QTREEWIDGET_ITEM("تاريخ الانشاء", QDateTime::fromTime_t(info->creatTime).toString("dd/MM/yyyy - hh:mm"));

        ADD_QTREEWIDGET_ITEM("مسار الفهرس", m_currentIndex->path());
        ADD_QTREEWIDGET_ITEM("مسار المكتبة الشاملة", m_currentIndex->shamelaPath());
        ADD_QTREEWIDGET_ITEM("عدد الصفحات", r->numDocs());

        //ADD_QTREEWIDGET_ITEM("Max Docs", r->maxDoc());
        //ADD_QTREEWIDGET_ITEM("Current Version", ver));

        TermEnum* te = r->terms();
        int32_t nterms = 0;

        for (nterms = 0; te->next() == true; nterms++) {}

        ADD_QTREEWIDGET_ITEM("عدد الكلمات", nterms);
        if(info) {
            ADD_QTREEWIDGET_ITEM("حجم الفهرس", getSizeString(info->indexSize));
            ADD_QTREEWIDGET_ITEM("حجم الكتب المفهرسة", getSizeString(info->shamelaSize));
            ADD_QTREEWIDGET_ITEM("مدة الفهرسة", getTimeString(info->indexingTime, false));
            ADD_QTREEWIDGET_ITEM("مدة ضغط الفهرس", getTimeString(info->optimizingTime, false));
        } else {
            ADD_QTREEWIDGET_ITEM("حجم الفهرس", getSizeString(getIndexSize(m_currentIndex->path())));
            ADD_QTREEWIDGET_ITEM("حجم الكتب المفهرسة", getSizeString(getBooksSize(m_currentIndex->shamelaPath())));
        }

        treeWidget->addTopLevelItems(itemList);
        treeWidget->resizeColumnToContents(1);
        treeWidget->resizeColumnToContents(0);

        QDialog *dialog = new QDialog(this);
        hideHelpButton(dialog);

        QVBoxLayout *layout = new QVBoxLayout();
        QLabel *label = new QLabel(trUtf8("معلومات حول الفهرس:"), dialog);
        layout->addWidget(label);
        layout->addWidget(treeWidget);

        dialog->setWindowTitle(trUtf8("%1 %2").arg(APP_NAME).arg(APP_VERSION_STR));
        dialog->setLayout(layout);
        dialog->resize(400, 300);
        dialog->show();

        _CLLDELETE(te);
        r->close();
        _CLLDELETE(r);
    }
    catch(CLuceneError &err) {
        QMessageBox::warning(0, "Error search", err.what());
    }
    catch(...) {
        qCritical("[%s:%d] Cannot open index at \"%s\".",
               __FILE__,
               __LINE__,
               qPrintable(m_currentIndex->path()));
    }

}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(this);
    connect(&dialog, SIGNAL(settingsUpdated()), m_searchWidget, SLOT(loadSettings()));

    dialog.exec();
}

void MainWindow::aboutApp()
{
    QString aTitle(trUtf8(" حول البرنامج"));
    QString aText(trUtf8("برنامج %1 للبحث في كتب المكتبة الشاملة"
                         "<br>"
                         "اصدار البرنامج: %2").arg(APP_NAME).arg(APP_VERSION_STR));

    QMessageBox::information(this, aTitle, aText);
}

void MainWindow::showLogDialog()
{    
    m_logDialog->startWatching();
    m_logDialog->show();
}
