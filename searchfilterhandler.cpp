#include "searchfilterhandler.h"
#include "shamelamodels.h"
#include <qsortfilterproxymodel.h>
#include <qmenu.h>
#include <qdebug.h>

SearchFilterHandler::SearchFilterHandler(QObject *parent) :
    QObject(parent)
{
    m_filterProxy = new QSortFilterProxyModel(this);
//    m_filterProxy->setDynamicSortFilter(true);

    m_menu = new QMenu(0);
    QAction *actionIgnore = new QAction(trUtf8("تجاهل الفروف في الهمزات ونحوها"), this);
    actionIgnore->setCheckable(true);
    actionIgnore->setChecked(true);

    m_menu->addAction(actionIgnore);
    m_menu->addSeparator();

    QAction *actionSelected = new QAction(trUtf8("عرض ما تم اختياره"), this);
    m_menu->addAction(actionSelected);

    QAction *actionUnSelected = new QAction(trUtf8("عرض ما لم يتم اختياره"), this);
    m_menu->addAction(actionUnSelected);

    m_ignore = true;

    connect(actionIgnore, SIGNAL(toggled(bool)), SLOT(ignoreSameChars(bool)));
    connect(actionSelected, SIGNAL(triggered()), SLOT(showSelected()));
    connect(actionUnSelected, SIGNAL(triggered()), SLOT(showUnSelected()));
}

SearchFilterHandler::~SearchFilterHandler()
{
    delete m_menu;
}

void SearchFilterHandler::setShamelaModels(ShamelaModels *shaModel)
{
    m_shaModel = shaModel;
}

void SearchFilterHandler::setFilterModel(QSortFilterProxyModel *filter)
{
    m_filterProxy = filter;
}

void SearchFilterHandler::setFilterText(QString text)
{
    m_filterText = text;

    if(m_ignore) {
        text.replace(QRegExp("[\\x0627\\x0622\\x0623\\x0625]"), "[\\x0627\\x0622\\x0623\\x0625]");//ALEFs
        text.replace(QRegExp("[\\x0647\\x0629]"), "[\\x0647\\x0629]"); //TAH_MARBUTA, HEH
        text.replace(QRegExp("[\\x062F\\x0630]"), "[\\x062F\\x0630]"); //DAL, THAL
    }

    m_filterProxy->setFilterRole(Qt::DisplayRole);
    m_filterProxy->setFilterRegExp(text);
}

void SearchFilterHandler::setFilterSourceModel(int index)
{
    m_index = index;
    chooseFilterSourceModel();
}

void SearchFilterHandler::chooseFilterSourceModel()
{
    QStandardItemModel *model;

    if(m_index == 0)
        model = m_shaModel->booksModel();

    else if(m_index == 1)
        model = m_shaModel->catsModel();

    else
        model = m_shaModel->authorsModel();

    m_filterProxy->setSourceModel(model);
}

QSortFilterProxyModel *SearchFilterHandler::getFilterModel()
{
    return m_filterProxy;
}

QMenu *SearchFilterHandler::getFilterLineMenu()
{
    return m_menu;
}

void SearchFilterHandler::ignoreSameChars(bool ignore)
{
    m_ignore = ignore;
    setFilterText(m_filterText);
}

void SearchFilterHandler::showSelected()
{
    QVariant checked(Qt::Checked);

    m_filterProxy->setFilterRole(Qt::CheckStateRole);
    m_filterProxy->setFilterFixedString(checked.toString());
}

void SearchFilterHandler::showUnSelected()
{
    QVariant unChecked(Qt::Unchecked);

    m_filterProxy->setFilterRole(Qt::CheckStateRole);
    m_filterProxy->setFilterFixedString(unChecked.toString());
}
