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
#include "searchquerywidget.h"
#include "networkrequest.h"
#include "shamelaindexinfo.h"

#include <qmessagebox.h>
#include <qsettings.h>
#include <qprogressdialog.h>
#include <qevent.h>
#include <qabstractitemmodel.h>
#include <qmenu.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>

ShamelaSearchWidget::ShamelaSearchWidget(QWidget *parent) :
    AbstractSearchWidget(parent),
    ui(new Ui::ShamelaSearchWidget)
{
    ui->setupUi(this);

    m_booksDB = new BooksDB();

    m_shaModel = new ShamelaModels(this);
    m_filterHandler = new SearchFilterHandler(this);
    m_nam = new QNetworkAccessManager(this);

    SelectedFilterWidget *selected = new SelectedFilterWidget(this);
    selected->hide();
    ui->widgetSelectedFilter->layout()->addWidget(selected);
    m_filterHandler->setSelectedFilterWidget(selected);

    if(m_filterHandler->getFilterLineMenu())
        ui->lineFilter->setFilterMenu(m_filterHandler->getFilterLineMenu());
    m_searchCount = 0;
    m_proccessItemChange = true;

    loadSettings();

    connect(ui->pushSearch, SIGNAL(clicked()), SLOT(search()));
    connect(ui->searchQueryWidget, SIGNAL(search()), SLOT(search()));
    connect(m_filterHandler, SIGNAL(clearText()), ui->lineFilter, SLOT(clear()));

    connect(ui->pushSelectAll, SIGNAL(clicked()), SLOT(selectAllBooks()));
    connect(ui->pushUnSelectAll, SIGNAL(clicked()), SLOT(unSelectAllBooks()));
    connect(ui->pushSelectVisible, SIGNAL(clicked()), SLOT(selectVisibleBooks()));
    connect(ui->pushUnselectVisible, SIGNAL(clicked()), SLOT(unSelectVisibleBooks()));
    connect(ui->pushExpandTree, SIGNAL(clicked()), SLOT(expandFilterView()));
    connect(ui->pushCollapseTree, SIGNAL(clicked()), SLOT(collapseFilterView()));
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

    ui->widgetSearchTools->setVisible(settings.value("Search/showSearchTools", false).toBool());

    if(m_resultParPage <= 0)
        m_resultParPage = 10;

    if(settings.value("Search/saveSearchOptions", false).toBool()) {
        int sort = settings.value("Search/sortBy", Relvance).toInt();
        ui->comboSortBy->setCurrentIndex(sort);

        int searchIn = settings.value("Search/searchIn", 0).toInt();
        ui->comboSearchIn->setCurrentIndex(searchIn);
    }

    ui->searchQueryWidget->loadSettings();
}

void ShamelaSearchWidget::saveSettings()
{
    qDebug("Save settings");

    QSettings settings;

    settings.setValue("resultPeerPage", m_resultParPage);
    settings.setValue("useTabs", m_useMultiTab);

    if(settings.value("Search/saveSearchOptions", false).toBool()) {
        int sort = ui->comboSortBy->currentIndex();
        settings.setValue("Search/sortBy", sort);

        int searchIn = ui->comboSearchIn->currentIndex();
        settings.setValue("Search/searchIn", searchIn);
    }

    ui->searchQueryWidget->saveSettings();
}

void ShamelaSearchWidget::search()
{
    ArabicAnalyzer analyzer;
    BooleanQuery *q = new BooleanQuery;
    Query *filterQuery = 0;
    QueryParser *queryPareser;

    if(ui->comboSearchIn->currentIndex() == 1) {
        queryPareser = new QueryParser(PAGE_TEXT_FIELD, &analyzer);
    } else if(ui->comboSearchIn->currentIndex() == 2) {
        queryPareser = new QueryParser(FOOT_NOTE_FIELD, &analyzer);
    } else {
        //BoostMap *boosts = new BoostMap();
        //boosts->put(PAGE_TEXT_FIELD, 1.0);
        //boosts->put(FOOT_NOTE_FIELD, 0.5);

        const TCHAR *fields[] = {PAGE_TEXT_FIELD, FOOT_NOTE_FIELD, NULL};
        queryPareser = new MultiFieldQueryParser(fields, &analyzer);
    }

    queryPareser->setAllowLeadingWildcard(true);

    Query *searchQuery = ui->searchQueryWidget->searchQuery(queryPareser);
    if(!searchQuery)
        return;

    q->add(searchQuery, BooleanClause::MUST);

    m_shaModel->generateLists();

    // Filtering
    bool required = m_shaModel->selectedBooksCount() <= m_shaModel->unSelectBooksCount();
    bool prohibited = !required;

    filterQuery = getBooksListQuery();

    if(filterQuery) {
        filterQuery->setBoost(0.0);
        q->add(filterQuery, required, prohibited);
    }

    ShamelaSearcher *m_searcher = new ShamelaSearcher;
    m_searcher->setBooksDb(m_booksDB);
    m_searcher->setIndexInfo(m_currentIndex);
    m_searcher->setQueryString(ui->searchQueryWidget->searchQueryStr(true));
    m_searcher->setQuery(q);

    m_searcher->setResultsPeerPage(m_resultParPage);
    m_searcher->setSortNum((SearchSort)ui->comboSortBy->currentIndex());

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

    // Send query to the server
    QUrl url("http://albahhet.sourceforge.net/bahhet.php");
    url.addQueryItem("query", ui->searchQueryWidget->searchQueryStr());
    url.addQueryItem("uid", userId());

    m_nam->get(NetworkRequest(url));
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

void ShamelaSearchWidget::setIndexInfo(IndexInfoBase *info)
{
    m_currentIndex = static_cast<ShamelaIndexInfo*>(info);
    m_booksDB->setIndexInfo(m_currentIndex);
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

    ui->groupSearchFilter->setTitle(tr("مجال البحث (الكتب: %1 - الأقسام: %2)")
                             .arg(m_booksDB->getBooksCount())
                             .arg(booksModel->rowCount()));

    progress.setValue(progress.maximum());

    connect(booksModel, SIGNAL(itemChanged(QStandardItem*)), SLOT(itemChanged(QStandardItem*)));
    connect(ui->treeViewBooks->header(),
            SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortChanged(int,Qt::SortOrder)));
}

void ShamelaSearchWidget::on_lineFilter_textChanged(QString text)
{
    if(text.size() > 2) {
        m_filterHandler->setFilterText(text);
        ui->treeViewBooks->expandAll();
    } else {
        m_filterHandler->setFilterText("");
        ui->treeViewBooks->collapseAll();
    }
}

void ShamelaSearchWidget::sortChanged(int logicalIndex, Qt::SortOrder)
{
    if(logicalIndex != 2)
        m_filterHandler->getFilterModel()->setSortRole(Qt::DisplayRole);
    else
        m_filterHandler->getFilterModel()->setSortRole(BooksDB::authorDeathRole);
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
        TCHAR *lineText = QueryParser::escape(QStringToTChar(edit->text()));
        edit->setText(QString::fromWCharArray(lineText));

        free(lineText);
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
