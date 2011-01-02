#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_currentIndex = new IndexInfo();

    m_fetechProgressBar = new QProgressBar(this);
    m_fetechProgressBar->setTextVisible(false);
    statusBar()->addWidget(m_fetechProgressBar, 0);
    m_fetechProgressBar->hide();

    m_searchTimeLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_searchTimeLabel, 0);
    m_searchTimeLabel->hide();

    m_searchResultsLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_searchResultsLabel, 1);
    m_searchTimeLabel->hide();

    m_searchCount = 0;
    m_resultParPage = 10;
    m_dbIsOpen = false;

    FORCE_RTL(ui->lineQueryMust);
    FORCE_RTL(ui->lineQueryShould);
    FORCE_RTL(ui->lineQueryShouldNot);

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    m_resultParPage = settings.value("resultPeerPage", m_resultParPage).toInt();

    settings.beginGroup("MainWindow");
    resize(settings.value("size", size()).toSize());
    move(settings.value("pos", pos()).toPoint());
    settings.endGroup();
    loadIndexesList();

    ui->lineQueryMust->setText(settings.value("lastQueryMust").toString());
    ui->lineQueryShould->setText(settings.value("lastQueryShould").toString());
    ui->lineQueryShouldNot->setText(settings.value("lastQueryShouldNot").toString());

    connect(ui->actionNewIndex, SIGNAL(triggered()), this, SLOT(newIndex()));
    connect(ui->pushSearch, SIGNAL(clicked()), this, SLOT(startSearching()));
    connect(ui->actionIndexInfo, SIGNAL(triggered()), this, SLOT(showStatistic()));
    connect(ui->actionSearchSettings, SIGNAL(triggered()), this, SLOT(displayResultsOptions()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabCountChange(int)));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveSettings()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.setValue("lastQueryMust", ui->lineQueryMust->text());
    settings.setValue("lastQueryShould", ui->lineQueryShould->text());
    settings.setValue("lastQueryShouldNot", ui->lineQueryShouldNot->text());

    settings.setValue("resultPeerPage", m_resultParPage);

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    if(!m_currentIndex->name().isEmpty())
        settings.setValue("current_index",
                          QString("i_%1").arg(IndexInfo::nameHash(m_currentIndex->name())));
}

void MainWindow::loadIndexesList()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    QStringList list =  settings.value("indexes_list").toStringList();
    bool enableWidgets = true;

    if(!list.isEmpty()) {
        QString current = settings.value("current_index", list.first()).toString();

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
            connect(action, SIGNAL(triggered()), this, SLOT(changeIndex()));

            ui->menuIndexesList->addAction(action);
        }

        selectIndex(current);

    } else {
        enableWidgets = false;
    }

    ui->tabWidget->setEnabled(enableWidgets);
}

void MainWindow::selectIndex(QString name)
{
    foreach(QAction *a, ui->menuIndexesList->actions()) {
        if(a->data().toString() == name) {
            selectIndex(a);
            break;
        }
    }

}

void MainWindow::selectIndex(QAction *act)
{
    m_currentIndex = m_indexInfoMap[act->data().toString()];
    act->setCheckable(true);
    act->setChecked(true);

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
        delete m_bookQuery;
        m_bookDB.close();
    }
    m_dbIsOpen = false;
}

void MainWindow::updateIndexesMenu()
{
    QList<QAction*> list = ui->menuIndexesList->actions();
    qDeleteAll(list);
    m_indexInfoMap.clear();

    loadIndexesList();
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void MainWindow::newIndex()
{
    IndexingDialg *indexDial = new IndexingDialg(this);
    if(indexDial->exec() == QDialog::Accepted)
       updateIndexesMenu();
}

void MainWindow::startSearching()
{
    if(!m_dbIsOpen)
        openDB();

    QString mustQureyStr = ui->lineQueryMust->text();
    QString shouldQureyStr = ui->lineQueryShould->text();
    QString shouldNotQureyStr = ui->lineQueryShouldNot->text();

    NORMALISE_SEARCH_STRING(mustQureyStr);
    NORMALISE_SEARCH_STRING(shouldQureyStr);
    NORMALISE_SEARCH_STRING(shouldNotQureyStr);

    m_searchQuery = mustQureyStr + " " + shouldQureyStr;

    ArabicAnalyzer analyzer;
    BooleanQuery *q = new BooleanQuery;
    QueryParser *queryPareser = new QueryParser(_T("text"), &analyzer);
    queryPareser->setAllowLeadingWildcard(true);

    if(!mustQureyStr.isEmpty()) {
        if(ui->checkQueryMust->isChecked())
            queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);
        else
            queryPareser->setDefaultOperator(QueryParser::OR_OPERATOR);

        Query *mq = queryPareser->parse(QSTRING_TO_TCHAR(mustQureyStr));
        q->add(mq, BooleanClause::MUST);

    }

    if(!shouldQureyStr.isEmpty()) {
        if(ui->checkQueryShould->isChecked())
            queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);
        else
            queryPareser->setDefaultOperator(QueryParser::OR_OPERATOR);

        Query *mq = queryPareser->parse(QSTRING_TO_TCHAR(shouldQureyStr));
        q->add(mq, BooleanClause::SHOULD);

    }

    if(!shouldNotQureyStr.isEmpty()) {
        if(ui->checkQueryShouldNot->isChecked())
            queryPareser->setDefaultOperator(QueryParser::AND_OPERATOR);
        else
            queryPareser->setDefaultOperator(QueryParser::OR_OPERATOR);

        Query *mq = queryPareser->parse(QSTRING_TO_TCHAR(shouldNotQureyStr));
        q->add(mq, BooleanClause::MUST_NOT);
    }

    ShamelaSearcher *m_searcher = new ShamelaSearcher;
    m_searcher->setIndexInfo(m_currentIndex);
    m_searcher->setQueryString(m_searchQuery);
    m_searcher->setQuery(q);
    m_searcher->setResultsPeerPage(m_resultParPage);

    ShamelaResultWidget *widget = new ShamelaResultWidget(this);
    widget->setShamelaSearch(m_searcher);
    widget->setIndexInfo(m_currentIndex);

    int index = ui->tabWidget->addTab(widget,
                                      trUtf8("%1 (%2)")
                                      .arg(m_currentIndex->name())
                                      .arg(++m_searchCount));
    ui->tabWidget->setCurrentIndex(index);

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
        int32_t nterms;

        bool writeToFile = false;

        if(writeToFile) {
            QFile logFile("terms.txt");
            if(logFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
                QTextStream log(&logFile);
                for (nterms = 0; te->next() == true; nterms++) {
                    /* if(_wcsicmp(te->term()->field(), _T("bookid")) == 0) */
                    log << TCHAR_TO_QSTRING(te->term()->toString()) << "\n";
                }
            }
        } else {
            for (nterms = 0; te->next() == true; nterms++) {
                /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
            }
        }

        ADD_QTREEWIDGET_ITEM("عدد الكلمات", nterms)
        ADD_QTREEWIDGET_ITEM("حجم الفهرس", getIndexSize())
        ADD_QTREEWIDGET_ITEM("حجم الكتب المفهرسة", getBooksSize())

        treeWidget->addTopLevelItems(itemList);
        treeWidget->resizeColumnToContents(1);
        treeWidget->resizeColumnToContents(0);

        QDialog *dialog = new QDialog(this);
        QVBoxLayout *layout = new QVBoxLayout();
        QLabel *label = new QLabel(trUtf8("معلومات حول الفهرس:"), dialog);
        layout->addWidget(label);
        layout->addWidget(treeWidget);

        dialog->setWindowTitle(windowTitle());
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
        qDebug() << "Error when opening : " << m_currentIndex->path();
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
                      .arg(m_currentIndex->shamelaDbPath());
    m_bookDB.setDatabaseName(mdbpath);

    if (!m_bookDB.open()) {
        QMessageBox::warning(0, "Error opening database", "Cannot open main.mdb database.");
        return false;
    }
    m_bookQuery = new QSqlQuery(m_bookDB);
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
        txt.append(tr("[+] Date: %1\n").arg(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:MM:ss")));
#ifdef GITVERSION
        txt.append(tr("[+] Git: %1 - Change number: %2\n").arg(GITVERSION).arg(GITCHANGENUMBER));
#endif
        txt.append(tr("[+] Statistics for \"%1\"\n").arg(m_currentIndex->path()));
        txt.append(tr("[+] Current Version: %1\n").arg(ver)) ;
        txt.append(tr("[+] Num Docs: %1\n").arg(r->numDocs()));

        TermEnum* te = r->terms();
        int32_t nterms;
        for (nterms = 0; te->next() == true; nterms++) {
            /* qDebug() << TCHAR_TO_QSTRING(te->term()->text()); */
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

void MainWindow::displayResultsOptions()
{
    QDialog *settingDialog =  new QDialog(this);
    settingDialog->setWindowTitle(windowTitle());
    QVBoxLayout *vLayout= new QVBoxLayout;
    QHBoxLayout *hLayout= new QHBoxLayout;
    QLabel *label = new QLabel(trUtf8("عدد النتائج في كل صفحة:"), settingDialog);
    QSpinBox *spinPage = new QSpinBox(settingDialog);
    QPushButton *pushDone = new QPushButton(trUtf8("حفظ"), settingDialog);

    spinPage->setMaximum(1000);
    spinPage->setMinimum(1);
    spinPage->setValue(m_resultParPage);
    hLayout->addWidget(label);
    hLayout->addWidget(spinPage);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(pushDone);
    settingDialog->setLayout(vLayout);
    settingDialog->show();

    connect(pushDone, SIGNAL(clicked()), settingDialog, SLOT(accept()));
    connect(spinPage, SIGNAL(valueChanged(int)), this, SLOT(setResultParPage(int)));
}

QString MainWindow::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(m_currentIndex->shamelaPath()).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(m_currentIndex->shamelaPath()).arg(archive);
}
