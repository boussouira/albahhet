#include "shamelasearchwidget.h"
#include "ui_shamelasearchwidget.h"

#include "common.h"
#include "shamelasearcher.h"
#include "shamelaresultwidget.h"
#include "shamelamodels.h"
#include "tabwidget.h"
#include "searchfilterhandler.h"
#include "shamelafilterproxymodel.h"
#include "selectedfilterwidget.h"

#include <qmessagebox.h>
#include <qsettings.h>
#include <qprogressdialog.h>
#include <qevent.h>
#include <qabstractitemmodel.h>
#include <qmenu.h>

ShamelaSearchWidget::ShamelaSearchWidget(QWidget *parent) :
    AbstractSearchWidget(parent),
    ui(new Ui::ShamelaSearchWidget)
{
    ui->setupUi(this);

    forceRTL(ui->lineQueryMust);
    forceRTL(ui->lineQueryShould);
    forceRTL(ui->lineQueryShouldNot);
    forceRTL(ui->lineFilter);

    m_shaModel = new ShamelaModels(this);
    m_filterHandler = new SearchFilterHandler(this);

    SelectedFilterWidget *selected = new SelectedFilterWidget(this);
    selected->hide();
    ui->widgetSelectedFilter->layout()->addWidget(selected);
    m_filterHandler->setSelectedFilterWidget(selected);

    if(m_filterHandler->getFilterLineMenu())
        ui->lineFilter->setMenu(m_filterHandler->getFilterLineMenu());
    m_searchCount = 0;
    m_proccessItemChange = true;

    loadSettings();
    setupCleanMenu();

    QSettings settings;
    ui->lineQueryMust->setText(settings.value("lastQueryMust").toString());
    ui->lineQueryShould->setText(settings.value("lastQueryShould").toString());
    ui->lineQueryShouldNot->setText(settings.value("lastQueryShouldNot").toString());

    connect(ui->lineQueryMust, SIGNAL(returnPressed()), SLOT(search()));
    connect(ui->lineQueryShould, SIGNAL(returnPressed()), SLOT(search()));
    connect(ui->lineQueryShouldNot, SIGNAL(returnPressed()), SLOT(search()));
    connect(ui->pushSearch, SIGNAL(clicked()), SLOT(search()));
    connect(ui->lineFilter, SIGNAL(textChanged(QString)), ui->treeViewBooks, SLOT(expandAll()));
    connect(m_filterHandler, SIGNAL(clearText()), ui->lineFilter, SLOT(clear()));
}

ShamelaSearchWidget::~ShamelaSearchWidget()
{
    delete ui;
}

void ShamelaSearchWidget::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void ShamelaSearchWidget::loadSettings()
{
    QSettings settings;
    m_resultParPage = settings.value("resultPeerPage", 10).toInt();
    m_useMultiTab = settings.value("useTabs", true).toBool();

    if(m_resultParPage <= 0)
        m_resultParPage = 10;
}

void ShamelaSearchWidget::saveSettings()
{
    qDebug("Save settings");

    QSettings settings;

    settings.setValue("lastQueryMust", ui->lineQueryMust->text());
    settings.setValue("lastQueryShould", ui->lineQueryShould->text());
    settings.setValue("lastQueryShouldNot", ui->lineQueryShouldNot->text());

    settings.setValue("resultPeerPage", m_resultParPage);
    settings.setValue("useTabs", m_useMultiTab);
}

void ShamelaSearchWidget::search()
{

    if(ui->lineQueryMust->text().isEmpty()){
        if(!ui->lineQueryShould->text().isEmpty()){
            ui->lineQueryMust->setText(ui->lineQueryShould->text());
            ui->lineQueryShould->clear();
        } else {
            QMessageBox::warning(this,
                                 tr("البحث"),
                                 tr("يجب ملء حقل العبارات التي يجب ان تظهر في النتائج"));
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
    Query *filterQuery = 0;
    QueryParser *queryPareser;

    if(ui->radioSearchText->isChecked()) {
        queryPareser = new QueryParser(PAGE_TEXT_FIELD, &analyzer);
    } else if(ui->radioSearchFN->isChecked()) {
        queryPareser = new QueryParser(FOOT_NOTE_FIELD, &analyzer);
    } else {
        //BoostMap *boosts = new BoostMap();
        //boosts->put(PAGE_TEXT_FIELD, 1.0);
        //boosts->put(FOOT_NOTE_FIELD, 0.5);

        const TCHAR *fields[] = {PAGE_TEXT_FIELD, FOOT_NOTE_FIELD, NULL};
        queryPareser = new MultiFieldQueryParser(fields, &analyzer);
    }

    queryPareser->setAllowLeadingWildcard(true);

    m_shaModel->generateLists();

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

        qDebug() << "Search:" << TCharToQString(q->toString(PAGE_TEXT_FIELD));

        // Filtering
        bool required = m_shaModel->selectedBooksCount() <= m_shaModel->unSelectBooksCount();
        bool prohibited = !required;

        filterQuery = getBooksListQuery();

        if(filterQuery) {
            filterQuery->setBoost(0.0);
            q->add(filterQuery, required, prohibited);
        }

    } catch(CLuceneError &e) {
        if(e.number() == CL_ERR_Parse)
            QMessageBox::warning(this,
                                 tr("خطأ في استعلام البحث"),
                                 tr("هنالك خطأ في احدى حقول البحث"
                                    "\n"
                                    "تأكد من حذف الأقواس و المعقوفات وغيرها،"
                                    " ويمكنك فعل ذلك من خلال زر التنظيف الموجود يسار حقل البحث، بعد الضغط على هذا الزر اعد البحث"
                                    "\n"
                                    "او تأكد من أنك تستخدمها بشكل صحيح"));
        else
            QMessageBox::warning(0,
                                 "CLucene Query error",
                                 tr("code: %1\nError: %2").arg(e.number()).arg(e.what()));

        _CLDELETE(q);
        _CLDELETE(queryPareser);

        return;
    }
    catch(...) {
        QMessageBox::warning(0,
                             "CLucene Query error",
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

    if      (ui->radioSortRelvance->isChecked())  m_searcher->setSortNum(Relvance);
    else if (ui->radioSortBookRelv->isChecked())  m_searcher->setSortNum(BookRelvance);
    else if (ui->radioSortBookPage->isChecked())  m_searcher->setSortNum(BookPage);
    else if (ui->radioSortDeathRelv->isChecked()) m_searcher->setSortNum(DeathRelvance);
    else if (ui->radioSortDeathBook->isChecked()) m_searcher->setSortNum(DeathBookPage);
    else m_searcher->setSortNum(Relvance);

    ShamelaResultWidget *widget;
    int index=1;
    QString title = tr("%1 (%2)")
                    .arg(m_currentIndex->name())
                    .arg(++m_searchCount);

    if(m_useMultiTab || m_tabWidget->count() < 2) {
        widget = new ShamelaResultWidget(this);
        index = m_tabWidget->addTab(widget, title);
    } else {
        widget = qobject_cast<ShamelaResultWidget*>(m_tabWidget->widget(index));
        widget->clearResults();
    }

    widget->setShamelaSearch(m_searcher);
    widget->setIndexInfo(m_currentIndex);
    widget->setBooksDb(m_booksDB);

    m_tabWidget->setCurrentIndex(index);
    m_tabWidget->setTabText(index, title);

    widget->doSearch();
}


Query *ShamelaSearchWidget::getBooksListQuery()
{
    int count = 0;

    // Every thing is selected we don't need a filter
    if(m_shaModel->unSelectBooksCount()==0 ||
            m_shaModel->selectedBooksCount()==0 ) {
        return 0;
    }

    QList<int> books;
    BooleanQuery *q = new BooleanQuery();
    q->setMaxClauseCount(0x7FFFFFFFL);

    if(m_shaModel->selectedBooksCount() <= m_shaModel->unSelectBooksCount()) {
        books = m_shaModel->selectedBooks();
    } else {
        books = m_shaModel->unSelectedBooks();
    }

    foreach(int id, books) {
        TermQuery *term = new TermQuery(new Term(BOOK_ID_FIELD, QStringToTChar(QString::number(id))));
        q->add(term, BooleanClause::SHOULD);
        count++;
    }

    return count ? q : 0;
}

void ShamelaSearchWidget::setIndexInfo(IndexInfo *info)
{
    m_currentIndex = info;
}

void ShamelaSearchWidget::setBooksDb(BooksDB *db)
{
    m_booksDB = db;
}

void ShamelaSearchWidget::setTabWidget(TabWidget *tabWidget)
{
    m_tabWidget = tabWidget;
}

void ShamelaSearchWidget::indexChanged()
{
    QProgressDialog progress(tr("جاري انشاء مجالات البحث..."), QString(), 0, 4, this);
    progress.setModal(Qt::WindowModal);

    QStandardItemModel *booksModel = m_booksDB->getBooksListModel();
    progress.setLabelText("انشاء لائحة الكتب");

    m_shaModel->setBooksListModel(booksModel);

    ui->treeViewBooks->setModel(booksModel);

    ui->treeViewBooks->expandAll();
    ui->treeViewBooks->resizeColumnToContents(0);
    ui->treeViewBooks->resizeColumnToContents(1);
    // Set the proxy model
    m_filterHandler->setShamelaModels(m_shaModel);
    ui->treeViewBooks->setModel(m_filterHandler->getFilterModel());

    progress.setValue(progress.maximum());

    connect(booksModel, SIGNAL(itemChanged(QStandardItem*)), SLOT(itemChanged(QStandardItem*)));
}

void ShamelaSearchWidget::on_lineFilter_textChanged(QString text)
{
    m_filterHandler->setFilterText(text);
}

void ShamelaSearchWidget::clearLineText()
{
    FancyLineEdit *edit = qobject_cast<FancyLineEdit*>(sender()->parent());

    if(edit) {
        edit->clear();
    }
}

void ShamelaSearchWidget::clearSpecialChar()
{
    FancyLineEdit *edit = qobject_cast<FancyLineEdit*>(sender()->parent());

    if(edit) {
        TCHAR *lineText = QueryParser::escape(QSTRING_TO_TCHAR(edit->text()));
        edit->setText(QString::fromWCharArray(lineText));

        free(lineText);
    }
}

void ShamelaSearchWidget::setupCleanMenu()
{
    QList<FancyLineEdit*> lines;
    lines << ui->lineQueryMust;
    lines << ui->lineQueryShould;
    lines << ui->lineQueryShouldNot;

    foreach(FancyLineEdit *line, lines) {
        QMenu *menu = new QMenu(line);
        QAction *clearTextAct = new QAction(tr("مسح النص"), line);
        QAction *clearSpecialCharAct = new QAction(tr("ابطال مفعول الاقواس وغيرها"), line);

        menu->addAction(clearTextAct);
        menu->addAction(clearSpecialCharAct);

        connect(clearTextAct, SIGNAL(triggered()), SLOT(clearLineText()));
        connect(clearSpecialCharAct, SIGNAL(triggered()), SLOT(clearSpecialChar()));

        line->setMenu(menu);
    }
}

void ShamelaSearchWidget::itemChanged(QStandardItem *item)
{
    if(item && m_proccessItemChange) {
        m_proccessItemChange = false;

        if(item->data(BooksDB::typeRole).toInt() == BooksDB::Categorie) {
            if(item->checkState() != Qt::PartiallyChecked) {
                for(int i=0; i<item->rowCount(); i++) {
                    item->child(i)->setCheckState(item->checkState());
                }
            }
        } else if(item->data(BooksDB::typeRole).toInt() == BooksDB::Book) {
            QStandardItem *parentItem = item->parent();
            int checkItems = 0;

            for(int i=0; i<parentItem->rowCount(); i++) {
                if(parentItem->child(i)->checkState()==Qt::Checked)
                    checkItems++;
            }

            if(checkItems == 0)
                parentItem->setCheckState(Qt::Unchecked);
            else if(checkItems < parentItem->rowCount())
                parentItem->setCheckState(Qt::PartiallyChecked);
            else
                parentItem->setCheckState(Qt::Checked);

        }

        m_proccessItemChange = true;
    }
}

void ShamelaSearchWidget::selectAllBooks()
{

    QAbstractItemModel *model = m_shaModel->booksModel();

    int rowCount = model->rowCount();
    QModelIndex topLeft = model->index(0, 0);
    QModelIndex bottomRight = model->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);

    foreach (QModelIndex index, selection.indexes()) {
        model->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}

void ShamelaSearchWidget::unSelectAllBooks()
{
    QAbstractItemModel *model = m_shaModel->booksModel();

    int rowCount = model->rowCount();
    QModelIndex topLeft = model->index(0, 0);
    QModelIndex bottomRight = model->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);

    foreach (QModelIndex index, selection.indexes()) {
        model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }
}

void ShamelaSearchWidget::selectVisibleBooks()
{
    QAbstractItemModel *model = m_filterHandler->getFilterModel();

    int rowCount = model->rowCount();
    QModelIndex topLeft = model->index(0, 0);
    QModelIndex bottomRight = model->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);

    foreach (QModelIndex index, selection.indexes()) {
        if(index.data(BooksDB::typeRole).toInt() == BooksDB::Categorie) {
            QModelIndex child = index.child(0, 0);

            while(child.isValid()) {
                if(child.data(BooksDB::typeRole).toInt() == BooksDB::Book)
                    model->setData(child, Qt::Checked, Qt::CheckStateRole);

                child = index.child(child.row()+1, 0);
            }
        }
    }
}

void ShamelaSearchWidget::unSelectVisibleBooks()
{
    QAbstractItemModel *model = m_filterHandler->getFilterModel();

    int rowCount = model->rowCount();
    QModelIndex topLeft = model->index(0, 0);
    QModelIndex bottomRight = model->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);

    foreach (QModelIndex index, selection.indexes()) {
        if(index.data(BooksDB::typeRole).toInt() == BooksDB::Categorie) {
            QModelIndex child = index.child(0, 0);

            while(child.isValid()) {
                if(child.data(BooksDB::typeRole).toInt() == BooksDB::Book)
                    model->setData(child, Qt::Unchecked, Qt::CheckStateRole);

                child = index.child(child.row()+1, 0);
            }
        }
    }
}

void ShamelaSearchWidget::expandFilterView()
{
    ui->treeViewBooks->expandAll();
}

void ShamelaSearchWidget::collapseFilterView()
{
    ui->treeViewBooks->collapseAll();
}

QList<int> ShamelaSearchWidget::selectedBooks()
{
    m_shaModel->generateLists();
    return m_shaModel->selectedBooks();
}

void ShamelaSearchWidget::selectBooks(QList<int> books)
{
    QAbstractItemModel *model = m_shaModel->booksModel();

    int rowCount = model->rowCount();
    QModelIndex topLeft = model->index(0, 0);
    QModelIndex bottomRight = model->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);

    foreach (QModelIndex index, selection.indexes()) {
        if(index.data(BooksDB::typeRole).toInt() == BooksDB::Categorie) {
            QModelIndex child = index.child(0, 0);

            while(child.isValid()) {
                if(child.data(BooksDB::typeRole).toInt() == BooksDB::Book) {
                    int bid = child.data(BooksDB::idRole).toInt();
                    model->setData(child,
                                   (books.contains(bid)) ? Qt::Checked : Qt::Unchecked,
                                   Qt::CheckStateRole);
                }
                child = index.child(child.row()+1, 0);
            }
        }
    }
}
