#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "common.h"
#include "indexingdialg.h"
#include "settingsdialog.h"
#include "indexesdialog.h"
#include "logdialog.h"
#include "tabwidget.h"
#include "abstractsearchwidget.h"
#include "shamelasearchwidget.h"
#include "quransearchwidget.h"
#include "searchfield.h"
#include "searchfieldsdialog.h"
#include "aboutdialog.h"
#include "updatedialog.h"
#include "app_version.h"

#include <qtextbrowser.h>
#include <qfile.h>
#include <qdir.h>
#include <qsettings.h>
#include <qlabel.h>
#include <qtreewidget.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qinputdialog.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME);

    m_indexesManager = new IndexesManager;

    m_tabWidget = new TabWidget(this);
    setCentralWidget(m_tabWidget);

    m_searchWidget = 0;
    m_currentIndex = 0;
    m_booksDB = new BooksDB();

    m_logDialog = new LogDialog(this);
    m_logDialog->hide();
    hideHelpButton(m_logDialog);

    m_searchFields = new SearchField();
    m_updateChecker = new UpdateChecker(this);

    loadSettings();

    connect(ui->actionNewIndex, SIGNAL(triggered()), SLOT(newIndex()));
    connect(ui->actionIndexInfo, SIGNAL(triggered()), SLOT(showStatistic()));
    connect(ui->actionSearchSettings, SIGNAL(triggered()), SLOT(showSettingsDialog()));
    connect(ui->actionEditIndexes, SIGNAL(triggered()), SLOT(editIndexes()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(aboutApp()));
    connect(ui->actionLogDialog, SIGNAL(triggered()), SLOT(showLogDialog()));
    connect(ui->actionSaveSelectedField, SIGNAL(triggered()), SLOT(saveSelectedField()));
    connect(ui->actionEditField, SIGNAL(triggered()), SLOT(searchfieldsDialog()));
    connect(ui->actionUpdate, SIGNAL(triggered()), m_updateChecker, SLOT(startCheck()));
    connect(m_updateChecker, SIGNAL(checkFinished()), SLOT(checkFinnished()));
}

MainWindow::~MainWindow()
{
    DELETE_DB(m_booksDB);
    delete m_searchFields;
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
        settings.setValue("currentIndex", m_currentIndex->id());

    if(m_searchWidget)
        m_searchWidget->saveSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings;

    m_showNewIndexMsg = settings.value("showNewIndexMsg", true).toBool();
    if(settings.value("Update/autoCheck", true).toBool()) {
        uint current = QDateTime::currentDateTime().toTime_t();
        uint lastCheck = settings.value("Update/lastCheck", 0).toUInt();
        if(current - lastCheck > 43200)
            QTimer::singleShot(30000, this, SLOT(autoUpdateCheck()));
    }

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
        current = settings.value("currentIndex", -1).toInt();
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

void MainWindow::loadSearchFields()
{
    QList<SearchFieldInfo*> fields = m_searchFields->getFieldsNames(m_currentIndex->id());

    // Delete existing search field
    foreach(QAction *act, ui->menuSearchFields->actions()) {
        if(act->data().toInt() > 0) {
            delete act;
        }
    }

    foreach(SearchFieldInfo *f, fields) {
        QAction *act = new QAction(this);
        act->setText(f->name);
        act->setData(f->fieldID);

        connect(act, SIGNAL(triggered()), SLOT(searchfieldSelected()));

        ui->menuSearchFields->addAction(act);
    }
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
            QMessageBox::critical(this, tr("تحميل فهرس"),
                                  tr("حدث خطأ عند تحميل الفهرس:"
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
    if(m_searchWidget)
        delete m_searchWidget;

    if(m_currentIndex->type() == IndexInfo::ShamelaIndex)
        m_searchWidget = new ShamelaSearchWidget(m_tabWidget);
    else if(m_currentIndex->type() == IndexInfo::QuranIndex)
        m_searchWidget = new QuranSearchWidget(m_tabWidget);
    else
        qFatal("Unknow index type");

    DELETE_DB(m_booksDB);

    m_booksDB = new BooksDB();
    m_booksDB->setIndexInfo(m_currentIndex);

    m_searchWidget->setIndexInfo(m_currentIndex);
    m_searchWidget->setBooksDb(m_booksDB);
    m_searchWidget->setTabWidget(m_tabWidget);
    m_searchWidget->indexChanged();

    m_tabWidget->insertTab(0,
                           m_searchWidget,
                           QIcon(":/data/images/find.png"),
                           tr("بحث"));

    m_tabWidget->setCurrentIndex(0);

    QSettings settings;

    if(m_currentIndex && m_currentIndex->id() != -1)
        settings.setValue("currentIndex", m_currentIndex->id());

    setWindowTitle(QString("%1 - %2").arg(APP_NAME).arg(m_currentIndex->name()));

    m_searchFields->setIndexInfo(m_currentIndex);

    loadSearchFields();

    connect(ui->actionSelectAllBooks, SIGNAL(triggered()),
            m_searchWidget, SLOT(selectAllBooks()));
    connect(ui->actionUnSelectallBooks, SIGNAL(triggered()),
            m_searchWidget, SLOT(unSelectAllBooks()));
    connect(ui->actionSelectVisibleBooks, SIGNAL(triggered()),
            m_searchWidget, SLOT(selectVisibleBooks()));
    connect(ui->actionUnselectVisisbleBooks, SIGNAL(triggered()),
            m_searchWidget, SLOT(unSelectVisibleBooks()));
    connect(ui->actionExpandFilter, SIGNAL(triggered()),
            m_searchWidget, SLOT(expandFilterView()));
    connect(ui->actionCollapseFilter, SIGNAL(triggered()),
            m_searchWidget, SLOT(collapseFilterView()));
}

void MainWindow::haveIndexesCheck()
{
    // Check if we have any index
    if(ui->menuIndexesList->actions().isEmpty() && m_showNewIndexMsg) {
        int rep = QMessageBox::question(this,
                                        tr("انشاء فهرس"),
                                        tr("لم يتم العثور على اي فهرس." "\n" "هل تريد انشاء فهرس جديد؟"),
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

        if(rep == QMessageBox::Yes)
            newIndex();
        else
            QMessageBox::information(this,
                                     tr("انشاء فهرس"),
                                     tr("يمكن انشاء فهرس جديد في اي وقت من خلال قائمة "
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

void MainWindow::saveSelectedField()
{
    QList<int> selectBooks = m_searchWidget->selectedBooks();
    if(!selectBooks.isEmpty()) {
        QString name = QInputDialog::getText(this,
                                             tr("حفظ مجال البحث"),
                                             tr("اسم مجال البحث:"));
        if(name.isEmpty()) {
            QMessageBox::warning(this,
                                 tr("حفظ مجال البحث"),
                                 tr("يجب ان تختار اسما لمجال البحث"));
        } else {
            if(m_searchFields->addField(m_currentIndex->id(), name, selectBooks)) {
                QMessageBox::information(this,
                                     tr("حفظ مجال البحث"),
                                     tr("تم حفظ مجال البحث"));

                loadSearchFields();
            }
        }
    } else {
        QMessageBox::warning(this,
                             tr("حفظ مجال البحث"),
                             tr("لم تقم باختيار اي كتاب!"));
    }
}

void MainWindow::searchfieldSelected()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(act) {
       QList<int> selectBooks = m_searchFields->getFieldBooks(act->data().toInt());
       m_searchWidget->selectBooks(selectBooks);
    }
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
    if(m_tabWidget->count() > 1) {
        QMessageBox::warning(this,
                             tr("تعديل الفهارس"),
                             tr("من فضلك قم باغلاق جميع نتائج البحث قبل تعديل الفهارس"));
        return;
    }

    IndexesDialog dialog(m_indexesManager, this);
    connect(&dialog, SIGNAL(indexesChanged()), SLOT(updateIndexesMenu()));

    dialog.exec();
}

void MainWindow::showStatistic()
{
#define ADD_VALUE(name, value) \
    textBrowser->append(QString("<strong>%1:</strong> <strong style=\"color:green\">%2</strong>").arg(tr(name)).arg(value));

    try {
        IndexReader* r = IndexReader::open(qPrintable(m_currentIndex->indexPath()));
        //int64_t ver = r->getCurrentVersion(qPrintable(m_currentIndex->path()));

        QTextBrowser *textBrowser = new QTextBrowser;
        IndexingInfo *info = m_currentIndex->indexingInfo();

        ADD_VALUE("اسم الفهرس", m_currentIndex->name());

        if(info)
            ADD_VALUE("تاريخ الانشاء", QDateTime::fromTime_t(info->creatTime).toString("dd/MM/yyyy - hh:mm"));

        ADD_VALUE("مسار الفهرس", m_currentIndex->path());
        ADD_VALUE("المكتبة الشاملة", m_currentIndex->shamelaPath());
        ADD_VALUE("عدد الصفحات", r->numDocs());

        //ADD_VALUE("Current Version", ver));

        TermEnum* te = r->terms();
        int32_t nterms = 0;

        for (nterms = 0; te->next() == true; nterms++) {}

        ADD_VALUE("عدد الكلمات", nterms);
        ADD_VALUE("حجم الفهرس", getSizeString(getIndexSize(m_currentIndex->indexPath())));
        ADD_VALUE("حجم المكتبة", getSizeString(getBooksSize(m_currentIndex->shamelaPath())));
        if(info) {
            ADD_VALUE("مدة الفهرسة", getTimeString(info->indexingTime, false));

            if(info->optimizingTime != -1)
                ADD_VALUE("مدة ضغط الفهرس", getTimeString(info->optimizingTime, false));
        }

        QDialog *dialog = new QDialog(this);
        hideHelpButton(dialog);

        QVBoxLayout *layout = new QVBoxLayout();
        QLabel *label = new QLabel(tr("معلومات حول الفهرس:"), dialog);
        layout->addWidget(label);
        layout->addWidget(textBrowser);

        dialog->setWindowTitle(tr("%1 %2").arg(APP_NAME).arg(APP_VERSION_STR));
        dialog->setLayout(layout);
        dialog->resize(350, 200);
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
//    QString aTitle(tr(" حول البرنامج"));
//    QString aText(tr("برنامج %1 للبحث في كتب المكتبة الشاملة"
//                     "<br>"
//                     "اصدار البرنامج: "
//                     "<span style=\"direction:ltr;\">%2</span>")
//                  .arg(APP_NAME)
//                  .arg(APP_VERSION_STR));

//    QMessageBox::information(this, aTitle, aText);
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::showLogDialog()
{
    m_logDialog->startWatching();
    m_logDialog->show();
}

void MainWindow::searchfieldsDialog()
{
    SearchFieldsDialog dialog(m_searchFields, this);
    dialog.exec();

    loadSearchFields();
}

void MainWindow::checkFinnished()
{
    UpdateInfo *info = m_updateChecker->result();

    if(!info) {
        if(m_updateChecker->autoCheck) {
            QSettings settings;
            settings.setValue("Update/lastCheck", QDateTime::currentDateTime().toTime_t());
        } else {
            if(m_updateChecker->hasError) {
                QMessageBox::information(this,
                                         tr("تحديث البرنامج"),
                                         tr("حدث خطأ اثناء البحث عن التحديث:" "\n")
                                         + m_updateChecker->errorString);
            } else {
                QMessageBox::information(this,
                                         tr("تحديث البرنامج"),
                                         tr("لا يوجد تحديث للبرنامج، انت تستخدم اخر اصدار"));
            }
        }
    } else {
        UpdateDialog dialog(this);
        dialog.setDownloadUrl(info);
        dialog.exec();
    }
}

void MainWindow::autoUpdateCheck()
{
    m_updateChecker->startCheck(true);
}
