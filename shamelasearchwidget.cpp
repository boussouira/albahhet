#include "shamelasearchwidget.h"
#include "ui_shamelasearchwidget.h"

#include "common.h"
#include "shamelasearcher.h"
#include "shamelaresultwidget.h"
#include "shamelamodels.h"
#include "tabwidget.h"
#include "searchfilterhandler.h"
#include "ShamelaFilterProxyModel.h"
#include "selectedfilterwidget.h"

#include <qmessagebox.h>
#include <qsettings.h>
#include <qprogressdialog.h>
#include <qevent.h>
#include <qabstractitemmodel.h>
#include <qmenu.h>

ShamelaSearchWidget::ShamelaSearchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShamelaSearchWidget)
{
    ui->setupUi(this);

    forceRTL(ui->lineQueryMust);
    forceRTL(ui->lineQueryShould);
    forceRTL(ui->lineQueryShouldNot);
    forceRTL(ui->lineFilter);

    m_shaModel = new ShamelaModels(this);
    m_filterHandler = new SearchFilterHandler(this);
    m_filterHandler->setShamelaModels(m_shaModel);

    SelectedFilterWidget *selected = new SelectedFilterWidget(this);
    selected->hide();
    ui->widgetSelectedFilter->layout()->addWidget(selected);
    m_filterHandler->setSelectedFilterWidget(selected);

    m_filterText << "" << "" << "";

    ui->lineFilter->setMenu(m_filterHandler->getFilterLineMenu());

    loadSettings();
    enableFilterWidget();
    setupCleanMenu();

    QSettings settings;
    ui->lineQueryMust->setText(settings.value("lastQueryMust").toString());
    ui->lineQueryShould->setText(settings.value("lastQueryShould").toString());
    ui->lineQueryShouldNot->setText(settings.value("lastQueryShouldNot").toString());

    connect(ui->lineQueryMust, SIGNAL(returnPressed()), SLOT(search()));
    connect(ui->lineQueryShould, SIGNAL(returnPressed()), SLOT(search()));
    connect(ui->lineQueryShouldNot, SIGNAL(returnPressed()), SLOT(search()));
    connect(ui->pushSearch, SIGNAL(clicked()), SLOT(search()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), SLOT(enableFilterWidget()));
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
    ui->comboBox->setCurrentIndex(settings.value("comboIndex", 0).toInt());

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
    settings.setValue("comboIndex", ui->comboBox->currentIndex());
}

void ShamelaSearchWidget::search()
{

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

    ArabicAnalyzer analyzer("C:\\Documents and Settings\\Administrateur\\Bureau\\stopwords.txt", "UTF-8");
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
        if(ui->comboBox->currentIndex() == 1) {
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
                                        "تأكد من حذف الأقواس و المعقوفات وغيرها، ويمكنك فعل ذلك من خلال زر التنظيف الموجود يسار حقل البحث، بعد الضغط على هذا الزر اعد البحث"
                                        "\n"
                                        "او تأكد من أنك تستخدمها بشكل صحيح"));
        else
            QMessageBox::warning(0,
                                 "CLucene Query error",
                                 tr("Error: %2\ncode: %1").arg(e.number()).arg(e.what()));

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

    ShamelaResultWidget *widget;
    int index=1;
    QString title = trUtf8("%1 (%2)")
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
    BooleanQuery *q = new BooleanQuery();
    foreach(int id, m_shaModel->getSelectedBooks()) {
        TermQuery *term = new TermQuery(new Term(_T("bookid"), QStringToTChar(QString::number(id))));
        q->add(term,  BooleanClause::SHOULD);
        count++;
    }

    return count ? q : NULL;
}

Query *ShamelaSearchWidget::getCatsListQuery()
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

Query *ShamelaSearchWidget::getAuthorsListQuery()
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

void ShamelaSearchWidget::setIndexInfo(IndexInfo *info)
{
    m_currentIndex = info;
}

void ShamelaSearchWidget::setBooksDb(BooksDB *db)
{
    m_booksDB = db;
    m_filterHandler->getFilterModel()->setBooksDb(db);
}

void ShamelaSearchWidget::setTabWidget(TabWidget *tabWidget)
{
    m_tabWidget = tabWidget;
}

void ShamelaSearchWidget::indexChanged()
{
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

void ShamelaSearchWidget::chooseProxy(int index)
{
    m_filterHandler->setFilterSourceModel(index);

    if(index == 0)
        ui->treeViewBooks->setModel(m_filterHandler->getFilterModel());

    else if(index == 1)
        ui->treeViewCats->setModel(m_filterHandler->getFilterModel());

    else
        ui->treeViewAuthors->setModel(m_filterHandler->getFilterModel());
}

void ShamelaSearchWidget::on_lineFilter_textChanged(QString text)
{
    m_filterText[ui->tabWidgetFilter->currentIndex()] = text;

    m_filterHandler->setFilterText(text);
}

void ShamelaSearchWidget::on_tabWidgetFilter_currentChanged(int index)
{
    chooseProxy(index);

    if(m_filterHandler->clearFilterOnChange() && !m_filterHandler->getFilterModel()->filterByAuthor())
        m_filterHandler->clearFilter();

    ui->lineFilter->setText(m_filterText.at(index));
    m_filterHandler->getFilterModel()->setFilterByAuthor(false);
}

void ShamelaSearchWidget::enableFilterWidget()
{
    if(ui->comboBox->currentIndex() == 1) {
        ui->groupBoxFilter->setEnabled(true);
    } else {
        ui->groupBoxFilter->setEnabled(false);
    }
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
        QAction *clearTextAct = new QAction(trUtf8("مسح النص"), line);
        QAction *clearSpecialCharAct = new QAction(trUtf8("ابطال مفعول الاقواس وغيرها"), line);

        menu->addAction(clearTextAct);
        menu->addAction(clearSpecialCharAct);

        connect(clearTextAct, SIGNAL(triggered()), SLOT(clearLineText()));
        connect(clearSpecialCharAct, SIGNAL(triggered()), SLOT(clearSpecialChar()));

        line->setMenu(menu);
    }
}

void ShamelaSearchWidget::on_treeViewAuthors_doubleClicked(QModelIndex index)
{
    if(index.isValid() && (ui->tabWidgetFilter->currentIndex() == 2)) {
        SelectedFilterWidget *selected = m_filterHandler->selectedFilterWidget();
        selected->setText(trUtf8("عرض كتب %1").arg(index.data().toString()));
        selected->show();
        m_filterHandler->getFilterModel()->setFilterByAuthor(true);
        m_filterHandler->getFilterModel()->setAuthor(index.data(Qt::UserRole).toInt());
        m_filterHandler->setClearFilterOnChange(true);
        ui->tabWidgetFilter->setCurrentIndex(0);
    }
}

void ShamelaSearchWidget::on_pushSelectAll_clicked()
{
    int rowCount =  m_filterHandler->getFilterModel()->rowCount();
    QModelIndex topLeft =  m_filterHandler->getFilterModel()->index(0, 0);
    QModelIndex bottomRight =  m_filterHandler->getFilterModel()->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);
    QItemSelection modelSelection =  m_filterHandler->getFilterModel()->mapSelectionToSource(selection);

    foreach (QModelIndex index, modelSelection.indexes()) {
        QStandardItemModel *model = m_shaModel->getModel(ui->tabWidgetFilter->currentIndex());
        model->setData(index, Qt::Checked, Qt::CheckStateRole);
    }
}

void ShamelaSearchWidget::on_pushUnSelectAll_clicked()
{
    int rowCount =  m_filterHandler->getFilterModel()->rowCount();
    QModelIndex topLeft =  m_filterHandler->getFilterModel()->index(0, 0);
    QModelIndex bottomRight =  m_filterHandler->getFilterModel()->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);
    QItemSelection modelSelection =  m_filterHandler->getFilterModel()->mapSelectionToSource(selection);

    foreach (QModelIndex index, modelSelection.indexes()) {
        QStandardItemModel *model = m_shaModel->getModel(ui->tabWidgetFilter->currentIndex());
        model->setData(index, Qt::Unchecked, Qt::CheckStateRole);
    }
}
