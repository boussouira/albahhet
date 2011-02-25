#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "common.h"
#include "indexinfo.h"
#include "indexingdialg.h"
#include "shamelasearcher.h"
#include "shamelaresultwidget.h"
#include "arabicanalyzer.h"
#include "settingsdialog.h"
#include "indexesdialog.h"
#include "shamelamodels.h"

#include <qtextbrowser.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qstandarditemmodel.h>
#include <qaction.h>
#include <qprogressbar.h>
#include <qlabel.h>
#include <qwebframe.h>
#include <qtreewidget.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qsqlquery.h>
#include <qprogressdialog.h>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME);

    m_currentIndex = new IndexInfo();
    m_booksDB = new BooksDB();
    m_shaModel = new ShamelaModels(this);

    m_filterHandler = new SearchFilterHandler(this);
    m_filterHandler->setShamelaModels(m_shaModel);

    m_filterText << "" << "" << "";

    m_searchCount = 0;
    m_resultParPage = 10;
    m_dbIsOpen = false;

    forceRTL(ui->lineQueryMust);
    forceRTL(ui->lineQueryShould);
    forceRTL(ui->lineQueryShouldNot);
    forceRTL(ui->lineFilter);

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    m_resultParPage = settings.value("resultPeerPage", m_resultParPage).toInt();
    m_useMultiTab = settings.value("useTabs", true).toBool();

    settings.beginGroup("MainWindow");
    resize(settings.value("size", size()).toSize());
    move(settings.value("pos", pos()).toPoint());
    settings.endGroup();

    ui->lineQueryMust->setText(settings.value("lastQueryMust").toString());
    ui->lineQueryShould->setText(settings.value("lastQueryShould").toString());
    ui->lineQueryShouldNot->setText(settings.value("lastQueryShouldNot").toString());

    ui->lineFilter->setMenu(m_filterHandler->getFilterLineMenu());

    connect(ui->actionNewIndex, SIGNAL(triggered()), SLOT(newIndex()));
    connect(ui->pushSearch, SIGNAL(clicked()), SLOT(startSearching()));
    connect(ui->actionIndexInfo, SIGNAL(triggered()), SLOT(showStatistic()));
    connect(ui->actionSearchSettings, SIGNAL(triggered()), SLOT(showSettingsDialog()));
    connect(ui->actionEditIndexes, SIGNAL(triggered()), SLOT(editIndexes()));
    connect(ui->actionAbout, SIGNAL(triggered()), SLOT(aboutApp()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(tabCountChange(int)));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
}

MainWindow::~MainWindow()
{
    deleteBooksDb(m_booksDB);
    delete ui;
}

void MainWindow::saveSettings()
{
    qDebug("Save settings");

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.setValue("lastQueryMust", ui->lineQueryMust->text());
    settings.setValue("lastQueryShould", ui->lineQueryShould->text());
    settings.setValue("lastQueryShouldNot", ui->lineQueryShouldNot->text());

    settings.setValue("resultPeerPage", m_resultParPage);
    settings.setValue("useTabs", m_useMultiTab);

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

    m_resultParPage = settings.value("resultPeerPage", m_resultParPage).toInt();
    m_useMultiTab = settings.value("useTabs", true).toBool();
}

void MainWindow::loadIndexesList()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    QStringList list =  settings.value("indexes_list").toStringList();
    bool haveIndexes = !list.isEmpty();

    if(haveIndexes) {
        QString current = settings.value("current_index").toString();
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

        selectIndex(current);
    }

    ui->tabWidget->setEnabled(haveIndexes);
    ui->actionIndexInfo->setEnabled(haveIndexes);
    ui->actionEditIndexes->setEnabled(haveIndexes);
    ui->menuIndexesList->setEnabled(haveIndexes);
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

    indexChanged();
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
    if(m_bookDB.isOpen()) {
//        delete m_bookQuery;
        m_bookDB.close();
    }
    m_dbIsOpen = false;

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    if(!m_currentIndex->name().isEmpty())
        settings.setValue("current_index", m_currentIndex->indexHash());

    setWindowTitle(QString("%1 - %2").arg(APP_NAME).arg(m_currentIndex->name()));

    deleteBooksDb(m_booksDB);

    m_booksDB = new BooksDB();
    m_booksDB->setIndexInfo(m_currentIndex);

    QProgressDialog progress(trUtf8("جاري انشاء مجالات البحث..."), QString(), 0, 4, this);
    progress.setModal(Qt::WindowModal);

    QStandardItemModel *catsModel = m_booksDB->getCatsListModel();
    PROGRESS_DIALOG_STEP("انشاء لائحة الأقسام");

    QStandardItemModel *booksModel = m_booksDB->getBooksListModel();
    PROGRESS_DIALOG_STEP("انشاء لائحة الكتب");

    QStandardItemModel *authModel = m_booksDB->getAuthorsListModel();
    PROGRESS_DIALOG_STEP("انشاء لائحة المؤلفيين");

    m_shaModel->setBooksListModel(booksModel);
    m_shaModel->setCatsListModel(catsModel);
    m_shaModel->setAuthorsListModel(authModel);

    ui->treeViewBooks->setModel(booksModel);
    ui->treeViewCats->setModel(catsModel);
    ui->treeViewAuthors->setModel(authModel);

    // Set the proxy model
    chooseProxy(ui->tabWidgetFilter->currentIndex());

    progress.setValue(progress.maximum());
}

void MainWindow::haveIndexesCheck()
{
    // Check if we have any index
    if(ui->menuIndexesList->actions().isEmpty()) {
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

void MainWindow::startSearching()
{
    if(!m_dbIsOpen)
        openDB();

    if(ui->lineQueryMust->text().isEmpty()){
        if(!ui->lineQueryShould->text().isEmpty()){
            ui->lineQueryMust->setText(ui->lineQueryShould->text());
            ui->lineQueryShould->clear();
        } else {
            QMessageBox::warning(this,
                                 trUtf8("البحث"),
                                 trUtf8("يجب ملء حقل العبارات التي يجب ان تظهر في النتائج"));
            return;
        }
    }

    QString mustQureyStr = ui->lineQueryMust->text();
    QString shouldQureyStr = ui->lineQueryShould->text();
    QString shouldNotQureyStr = ui->lineQueryShouldNot->text();

    normaliseSearchString(mustQureyStr);
    normaliseSearchString(shouldQureyStr);
    normaliseSearchString(shouldNotQureyStr);

    m_searchQuery = mustQureyStr + " " + shouldQureyStr;

    ArabicAnalyzer analyzer;
    BooleanQuery *q = new BooleanQuery;
    BooleanQuery *allFilterQuery = new BooleanQuery;
    QueryParser *queryPareser = new QueryParser(_T("text"), &analyzer);
    queryPareser->setAllowLeadingWildcard(true);

    try {
        if(!mustQureyStr.isEmpty()) {
            if(ui->checkQueryMust->isChecked())
                queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);
            else
                queryPareser->setDefaultOperator(QueryParser::OR_OPERATOR);

            Query *mq = queryPareser->parse(QStringToTChar(mustQureyStr));
            q->add(mq, BooleanClause::MUST);

        }

        if(!shouldQureyStr.isEmpty()) {
            if(ui->checkQueryShould->isChecked())
                queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);
            else
                queryPareser->setDefaultOperator(QueryParser::OR_OPERATOR);

            Query *mq = queryPareser->parse(QStringToTChar(shouldQureyStr));
            q->add(mq, BooleanClause::SHOULD);

        }

        if(!shouldNotQureyStr.isEmpty()) {
            if(ui->checkQueryShouldNot->isChecked())
                queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);
            else
                queryPareser->setDefaultOperator(QueryParser::OR_OPERATOR);

            Query *mq = queryPareser->parse(QStringToTChar(shouldNotQureyStr));
            q->add(mq, BooleanClause::MUST_NOT);
        }

        // Filtering
        if(ui->groupBoxFilter->isChecked()) {
            Query * filterQuery;
            bool required = ui->radioRequired->isChecked();
            bool prohibited = ui->radioProhibited->isChecked();
            bool gotAFilter = false;

            filterQuery = getBooksListQuery();
            if(filterQuery != NULL) {
                allFilterQuery->add(filterQuery, BooleanClause::SHOULD);
                gotAFilter = true;
            }

            filterQuery = getCatsListQuery();
            if(filterQuery != NULL) {
                allFilterQuery->add(filterQuery, BooleanClause::SHOULD);
                gotAFilter = true;
            }

            filterQuery = getAuthorsListQuery();
            if(filterQuery != NULL) {
                allFilterQuery->add(filterQuery, BooleanClause::SHOULD);
                gotAFilter = true;
            }

            if(gotAFilter) {
                allFilterQuery->setBoost(0.0);
                q->add(allFilterQuery, required, prohibited);
            }
        }
    } catch(CLuceneError &e) {
        if(e.number() == CL_ERR_Parse)
            QMessageBox::warning(this,
                                 trUtf8("خطأ في استعلام البحث"),
                                 trUtf8("هنالك خطأ في احدى حقول البحث"
                                        "\n"
                                        "تأكد من حذف الأقواس و المعقوفات وغيرها..."
                                        "\n"
                                        "او تأكد من أنك تستخدمها بشكل صحيح"));
        else
            QMessageBox::warning(0,
                                 "CLucene Error when Indexing",
                                 tr("Error code: %1\n%2").arg(e.number()).arg(e.what()));

        _CLDELETE(q);
        _CLDELETE(queryPareser);

        return;
    }
    catch(...) {
        QMessageBox::warning(0,
                             "Unkonw error when Indexing",
                             tr("Unknow error"));
        _CLDELETE(q);
        _CLDELETE(queryPareser);

        return;
    }

    ShamelaSearcher *m_searcher = new ShamelaSearcher;
    m_searcher->setBooksDb(m_booksDB);
    m_searcher->setIndexInfo(m_currentIndex);
    m_searcher->setQueryString(m_searchQuery);
    m_searcher->setQuery(q);
    m_searcher->setResultsPeerPage(m_resultParPage);

    ShamelaResultWidget *widget;
    int index=1;
    QString title = trUtf8("%1 (%2)")
                    .arg(m_currentIndex->name())
                    .arg(++m_searchCount);

    if(m_useMultiTab || ui->tabWidget->count() < 2) {
        widget = new ShamelaResultWidget(this);
        index = ui->tabWidget->addTab(widget, title);
    } else {
        widget = qobject_cast<ShamelaResultWidget*>(ui->tabWidget->widget(index));
        widget->clearResults();
    }

    widget->setShamelaSearch(m_searcher);
    widget->setIndexInfo(m_currentIndex);

    ui->tabWidget->setCurrentIndex(index);
    ui->tabWidget->setTabText(index, title);

    widget->doSearch();
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

void MainWindow::resultsCount()
{
}

void MainWindow::on_lineQueryMust_returnPressed()
{
    startSearching();
}

void MainWindow::on_lineQueryShould_returnPressed()
{
    startSearching();
}

void MainWindow::on_lineQueryShouldNot_returnPressed()
{
    startSearching();
}

void MainWindow::tabCountChange(int /*count*/)
{
    ui->tabWidget->setTabsClosable(ui->tabWidget->count() > 1);
}

void MainWindow::closeTab(int index)
{
    if(index > 0) {
        QWidget *w = ui->tabWidget->widget(index);
        ui->tabWidget->removeTab(index);
        delete w;
    }
}

bool MainWindow::openDB()
{
    m_dbIsOpen = false;

    m_bookDB = QSqlDatabase::addDatabase("QODBC", "shamelaBook");
    QString mdbpath = QString("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=%1")
                      .arg(m_currentIndex->shamelaMainDbPath());
    m_bookDB.setDatabaseName(mdbpath);

    if (!m_bookDB.open()) {
        DB_OPEN_ERROR(m_currentIndex->shamelaMainDbPath());
        return false;
    }

    m_dbIsOpen = true;

    return true;
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

void MainWindow::doneIndexing(int indexingTime)
{
    try {
        IndexReader* r = IndexReader::open(qPrintable(m_currentIndex->path()));
        int64_t ver = r->getCurrentVersion(qPrintable(m_currentIndex->path()));

        QString txt;
        txt.append(tr("[+] Date: %1\n")
                   .arg(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:MM:ss")));
#ifdef GITVERSION
        txt.append(tr("[+] Git: %1 - Change number: %2\n").arg(GITVERSION).arg(GITCHANGENUMBER));
#endif
        txt.append(tr("[+] Statistics for \"%1\"\n").arg(m_currentIndex->path()));
        txt.append(tr("[+] Current Version: %1\n").arg(ver)) ;
        txt.append(tr("[+] Num Docs: %1\n").arg(r->numDocs()));

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* qDebug() << TCharToQString(te->term()->text()); */
        }
        txt.append(tr("[+] Indexing took: %1 s\n").arg(indexingTime/1000.0));
        txt.append(tr("[+] Term count: %1\n").arg(nterms));
        txt.append(tr("[+] Index size: %1\n").arg(getIndexSize()));
        txt.append(tr("[+] Books size: %1\n").arg(getBooksSize()));
        txt.append("\n");
        _CLLDELETE(te);

        r->close();
        _CLLDELETE(r);

        QFile logFile("statistic.txt");
        if(logFile.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text)) {
             QTextStream log(&logFile);
             log << txt;
        }
    }
    catch(...) {}

}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(this);
    connect(&dialog, SIGNAL(settingsUpdated()), SLOT(loadSettings()));

    dialog.exec();
}

QString MainWindow::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb")
        .arg(m_currentIndex->shamelaPath())
        .arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb")
                .arg(m_currentIndex->shamelaPath())
                .arg(archive);
}

void MainWindow::aboutApp()
{
    QString aTitle(trUtf8(" حول البرنامج"));
    QString aText(trUtf8("برنامج %1 للبحث في كتب المكتبة الشاملة"
                         "<br>"
                         "اصدار البرنامج: %2").arg(APP_NAME).arg(APP_VERSION));

    QMessageBox::information(this, aTitle, aText);
}

Query *MainWindow::getBooksListQuery()
{
    int count = 0;
    BooleanQuery *q = new BooleanQuery();
    foreach(int id, m_shaModel->getSelectedBooks()) {
        TermQuery *term = new TermQuery(new Term(_T("bookid"), QStringToTChar(QString::number(id))));
        q->add(term,  BooleanClause::SHOULD);
        count++;
    }

    return count ? q : NULL;
}

Query *MainWindow::getCatsListQuery()
{
    int count = 0;
    BooleanQuery *q = new BooleanQuery();
    foreach(int id, m_shaModel->getSelectedCats()) {
        TermQuery *term = new TermQuery(new Term(_T("cat"), QStringToTChar(QString::number(id))));
        q->add(term,  BooleanClause::SHOULD);
        count++;
    }

    return count ? q : NULL;
}

Query *MainWindow::getAuthorsListQuery()
{
    int count = 0;
    BooleanQuery *q = new BooleanQuery();
    foreach(int id, m_shaModel->getSelectedAuthors()) {
        TermQuery *term = new TermQuery(new Term(_T("author"), QStringToTChar(QString::number(id))));
        q->add(term, BooleanClause::SHOULD);
        count++;
    }

    return count ? q : NULL;
}

void MainWindow::chooseProxy(int index)
{
    m_filterHandler->setFilterSourceModel(index);

    if(index == 0)
        ui->treeViewBooks->setModel(m_filterHandler->getFilterModel());

    else if(index == 1)
        ui->treeViewCats->setModel(m_filterHandler->getFilterModel());

    else
        ui->treeViewAuthors->setModel(m_filterHandler->getFilterModel());
}

void MainWindow::on_lineFilter_textChanged(QString text)
{
    m_filterText[ui->tabWidgetFilter->currentIndex()] = text;

    m_filterHandler->setFilterText(text);
}

void MainWindow::on_tabWidgetFilter_currentChanged(int index)
{
    chooseProxy(index);

    ui->lineFilter->setText(m_filterText.at(index));
}

