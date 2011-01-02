#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QWebFrame"

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

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    m_searchQuery = settings.value("lastQuery").toString();
    m_resultParPage = settings.value("resultPeerPage", m_resultParPage).toInt();

    settings.beginGroup("MainWindow");
    resize(settings.value("size", size()).toSize());
    move(settings.value("pos", pos()).toPoint());
    settings.endGroup();
    loadIndexesList();

    ui->lineQuery->setText(m_searchQuery);

    ui->lineQuery->setLayoutDirection(Qt::LeftToRight);
    ui->lineQuery->setLayoutDirection(Qt::RightToLeft);

    connect(ui->actionNewIndex, SIGNAL(triggered()), this, SLOT(newIndex()));
    connect(ui->pushSearch, SIGNAL(clicked()), this, SLOT(startSearching()));
    connect(ui->actionIndexInfo, SIGNAL(triggered()), this, SLOT(showStatistic()));
    connect(ui->actionSearchSettings, SIGNAL(triggered()), this, SLOT(displayResultsOptions()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabCountChange(int)));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

MainWindow::~MainWindow()
{
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.setValue("lastQuery", ui->lineQuery->text());
    settings.setValue("resultPeerPage", m_resultParPage);

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();

    if(!m_currentIndex->name().isEmpty())
        settings.setValue("current_index",
                          QString("i_%1").arg(IndexInfo::nameHash(m_currentIndex->name())));

    delete ui;
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

    m_searchQuery = ui->lineQuery->text();

    m_searchQuery.replace(QRegExp(trUtf8("ـفق")), "(");
    m_searchQuery.replace(QRegExp(trUtf8("ـغق")), ")");
    m_searchQuery.replace(QRegExp(trUtf8("ـ[أا]و")), "OR");
    m_searchQuery.replace(QRegExp(trUtf8("ـو")), "AND");
    m_searchQuery.replace(QRegExp(trUtf8("ـبدون")), "NOT");
    m_searchQuery.replace(trUtf8("؟"), "?");

    ShamelaSearcher *m_searcher = new ShamelaSearcher;
    m_searcher->setIndexInfo(m_currentIndex);
    m_searcher->setQueryString(m_searchQuery);
    m_searcher->setResultsPeerPage(m_resultParPage);
    m_searcher->setsetDefaultOperator(ui->checkBox->isChecked());

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
//        int64_t ver = r->getCurrentVersion(qPrintable(m_currentIndex->path()));

        QString txt("<p dir=\"rtl\" style=\"padding:5px;font-weight:bold;\">");

        txt.append(trUtf8("اسم الفهرس: <strong style=\"color:green;\">%1</strong>" "<br />").arg(m_currentIndex->name()));
        txt.append(trUtf8("مسار الفهرس: <strong style=\"color:green;\">%1</strong>" "<br />").arg(m_currentIndex->path()));
        txt.append(trUtf8("مسار المكتبة الشاملة: <strong style=\"color:green;\">%1</strong>" "<br />").arg(m_currentIndex->shamelaPath()));
        //txt.append(tr("Max Docs: %1<br/>").arg(r->maxDoc()));
//        txt.append(tr("Current Version: %1<br/><br/>").arg(ver)) ;
        txt.append(trUtf8("عدد الصفحات: <strong style=\"color:green;\">%1</strong>" "<br />").arg(r->numDocs()));

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

        txt.append(trUtf8("عدد الكلمات: <strong style=\"color:green;\">%1</strong>" "<br />").arg(nterms));
        txt.append(trUtf8("حجم الفهرس: <strong style=\"color:green;\">%1</strong>" "<br />").arg(getIndexSize()));
        txt.append(trUtf8("حجم الكتب المفهرسة: <strong style=\"color:green;\">%1</strong>" "<br />").arg(getBooksSize()));
        txt.append("</p>");

        QDialog *dialog = new QDialog(this);
        QVBoxLayout *layout = new QVBoxLayout();
        QTextBrowser *browser = new QTextBrowser(dialog);
        browser->setHtml(txt);
        QLabel *label = new QLabel(trUtf8(":معلومات حول الفهرس"), dialog);
        label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
        layout->addWidget(label);
        layout->addWidget(browser);

        dialog->setWindowTitle(windowTitle());
        dialog->setLayout(layout);
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

void MainWindow::on_lineQuery_returnPressed()
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
    QVBoxLayout *vLayout= new QVBoxLayout;
    QHBoxLayout *hLayout= new QHBoxLayout;
    QLabel *label = new QLabel(trUtf8(":عدد النتائج في كل صفحة"), settingDialog);
    QSpinBox *spinPage = new QSpinBox(settingDialog);
    QPushButton *pushDone = new QPushButton(trUtf8("حفظ"), settingDialog);

    spinPage->setMaximum(1000);
    spinPage->setMinimum(1);
    spinPage->setValue(m_resultParPage);
    hLayout->addWidget(spinPage);
    hLayout->addWidget(label);
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
