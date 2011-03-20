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

    m_tabWidget = new TabWidget(this);
    m_searchWidget = new ShamelaSearchWidget(m_tabWidget);
    m_searchWidget->setTabWidget(m_tabWidget);

    m_tabWidget->addTab(m_searchWidget,
                        QIcon(":/bin/data/images/find.png"),
                        trUtf8("بحث"));
    setCentralWidget(m_tabWidget);

    m_currentIndex = new IndexInfo();
    m_booksDB = new BooksDB();

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
    deleteBooksDb(m_booksDB);
    delete ui;
}

void MainWindow::saveSettings()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    if(!m_currentIndex->name().isEmpty())
        settings.setValue("current_index", m_currentIndex->indexHash());
}

void MainWindow::loadSettings()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    m_showNewIndexMsg = settings.value("showNewIndexMsg", true).toBool();

    settings.beginGroup("MainWindow");
    resize(settings.value("size", size()).toSize());
    move(settings.value("pos", pos()).toPoint());
    settings.endGroup();
}

void MainWindow::loadIndexesList()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    QStringList list =  settings.value("indexes_list").toStringList();
    QString current;
    bool haveIndexes = !list.isEmpty();

    if(haveIndexes) {
        current = settings.value("current_index").toString();
        if(current.isEmpty() || !list.contains(current))
            current = list.first();

        for(int i=0; i<list.count(); i++) {
            IndexInfo *info = new IndexInfo();
            settings.beginGroup(list.at(i));
            info->setName(settings.value("name").toString());
            info->setShamelaPath(settings.value("shamela_path").toString());
            info->setPath(settings.value("index_path").toString());
            info->setRamSize(settings.value("ram_size").toInt());
            info->setOptimizeIndex(settings.value("optimizeIndex").toBool());
            settings.endGroup();

            m_indexInfoMap.insert(list.at(i), info);

            QAction *action = new QAction(info->name(), ui->menuIndexesList);
            action->setData(list.at(i));
            connect(action, SIGNAL(triggered()), SLOT(changeIndex()));

            ui->menuIndexesList->addAction(action);
        }
    }

    m_tabWidget->setEnabled(haveIndexes);
    ui->actionIndexInfo->setEnabled(haveIndexes);
    ui->actionEditIndexes->setEnabled(haveIndexes);
    ui->menuIndexesList->setEnabled(haveIndexes);

    if(haveIndexes)
        selectIndex(current);
}

void MainWindow::selectIndex(QString name)
{
    foreach(QAction *action, ui->menuIndexesList->actions()) {
        if(action->data().toString() == name) {
            selectIndex(action);
            break;
        }
    }

}

void MainWindow::selectIndex(QAction *action)
{
    m_currentIndex = m_indexInfoMap[action->data().toString()];
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
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    if(!m_currentIndex->name().isEmpty())
        settings.setValue("current_index", m_currentIndex->indexHash());

    setWindowTitle(QString("%1 - %2").arg(APP_NAME).arg(m_currentIndex->name()));

    deleteBooksDb(m_booksDB);

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
    QList<QAction*> list = ui->menuIndexesList->actions();

    if(!list.isEmpty())
        qDeleteAll(list);

    m_indexInfoMap.clear();

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
    connect(&indexDialog, SIGNAL(indexCreated()), SLOT(updateIndexesMenu()));

    indexDialog.exec();
}

void MainWindow::editIndexes()
{
    IndexesDialog dialog(this);
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

        ADD_QTREEWIDGET_ITEM("اسم الفهرس", m_currentIndex->name())
        ADD_QTREEWIDGET_ITEM("مسار الفهرس", m_currentIndex->path())
        ADD_QTREEWIDGET_ITEM("مسار المكتبة الشاملة", m_currentIndex->shamelaPath())
        ADD_QTREEWIDGET_ITEM("عدد الصفحات", r->numDocs())

        //ADD_QTREEWIDGET_ITEM("Max Docs", r->maxDoc());
        //ADD_QTREEWIDGET_ITEM("Current Version", ver));

        TermEnum* te = r->terms();
        int32_t nterms = 0;

        bool writeToFile = false;

        if(writeToFile) {
            QFile logFile("terms.txt");
            if(logFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
                QTextStream log(&logFile);
                for (nterms = 0; te->next() == true; nterms++) {
                    /* if(_wcsicmp(te->term()->field(), _T("bookid")) == 0) */
                    log << TCharToQString(te->term()->toString()) << "\n";
                }
            }
        } else {
            for (nterms = 0; te->next() == true; nterms++) {
                /* qDebug() << TCharToQString(te->term()->text()); */
            }
        }

        ADD_QTREEWIDGET_ITEM("عدد الكلمات", nterms)
        ADD_QTREEWIDGET_ITEM("حجم الفهرس", getIndexSize())
        ADD_QTREEWIDGET_ITEM("حجم الكتب المفهرسة", getBooksSize())

        treeWidget->addTopLevelItems(itemList);
        treeWidget->resizeColumnToContents(1);
        treeWidget->resizeColumnToContents(0);

        QDialog *dialog = new QDialog(this);
        hideHelpButton(dialog);

        QVBoxLayout *layout = new QVBoxLayout();
        QLabel *label = new QLabel(trUtf8("معلومات حول الفهرس:"), dialog);
        layout->addWidget(label);
        layout->addWidget(treeWidget);

        dialog->setWindowTitle(APP_NAME);
        dialog->setLayout(layout);
        dialog->resize(treeWidget->sizeHint());
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

QString MainWindow::getIndexSize()
{
    QDir dir;
    dir.cd(m_currentIndex->path());
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    QString sizeStr;
    qint64 size = 0;
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        size += fileInfo.size();
    }

    if(size < 1024)
        sizeStr = trUtf8("%1 بيت").arg(size);
    else if(1024 <= size && size < 1024*1024)
        sizeStr = trUtf8("%1 كيلو").arg(size/(1024.0), 4);
    else if( 1024*1024 <= size && size < 1024*1024*1024)
        sizeStr = trUtf8("%1 ميغا").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = trUtf8("%1 جيجا").arg(size/(1024.0*1024.0*1024.0), 4);

    return sizeStr;
}

QString MainWindow::getBooksSize()
{
    QString sizeStr;
    qint64 size = getDirSize(m_currentIndex->shamelaPath());

    if(size < 1024)
        sizeStr = trUtf8("%1 بيت").arg(size);
    else if(1024 <= size && size < 1024*1024)
        sizeStr = trUtf8("%1 كيلو").arg(size/(1024.0), 4);
    else if( 1024*1024 <= size && size < 1024*1024*1024)
        sizeStr = trUtf8("%1 ميغا").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = trUtf8("%1 جيجا").arg(size/(1024.0*1024.0*1024.0), 4);

    return sizeStr;
}

qint64 MainWindow::getDirSize(const QString &path)
{
    QFileInfo info(path);
    qint64 size=0;

    if(info.isDir()){
        QDir dir(path);
        foreach(QFileInfo fieInfo, dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
            if(fieInfo.isFile() && fieInfo.suffix() == "mdb")
                size += fieInfo.size();
            else if(fieInfo.isDir())
                size += getDirSize(fieInfo.absoluteFilePath());
        }
    }

    return size;
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(this);
    connect(&dialog, SIGNAL(settingsUpdated()), SLOT(loadSettings()));
    connect(&dialog, SIGNAL(settingsUpdated()), m_searchWidget, SLOT(loadSettings()));

    dialog.exec();
}

void MainWindow::aboutApp()
{
    QString aTitle(trUtf8(" حول البرنامج"));
    QString aText(trUtf8("برنامج %1 للبحث في كتب المكتبة الشاملة"
                         "<br>"
                         "اصدار البرنامج: %2").arg(APP_NAME).arg(APP_VERSION));

    QMessageBox::information(this, aTitle, aText);
}

void MainWindow::showLogDialog()
{
    LogDialog dialog(this);
    hideHelpButton(&dialog);

    dialog.exec();
}
