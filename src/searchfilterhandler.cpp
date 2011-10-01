#include "searchfilterhandler.h"
#include "shamelamodels.h"
#include "shamelafilterproxymodel.h"
#include "selectedfilterwidget.h"
#include <qmenu.h>
#include <QItemSelection>
#include <qdebug.h>

SearchFilterHandler::SearchFilterHandler(QObject *parent) :
    QObject(parent)
{
    m_filterProxy = new ShamelaFilterProxyModel(this);
//    m_filterProxy->setDynamicSortFilter(true);

    m_menu = new QMenu(0);

    QAction *actionClearText = new QAction(tr("مسح النص"), this);
    m_menu->addAction(actionClearText);

    QAction *actionSelected = new QAction(tr("عرض ما تم اختياره"), this);
    m_menu->addAction(actionSelected);

    m_menu->addSeparator();

    QMenu *menu2 =  m_menu->addMenu(tr("بحث في"));
    m_actFilterByBooks = menu2->addAction(tr("اسماء الكتب"));
    m_actFilterByAuthors = menu2->addAction(tr("اسماء المؤلفين"));

    m_actFilterByBooks->setCheckable(true);
    m_actFilterByBooks->setChecked(true);
    m_actFilterByAuthors->setCheckable(true);

    m_hideFilterWidgetOnChange = false;

    connect(m_actFilterByBooks, SIGNAL(toggled(bool)), SLOT(filterByBooks(bool)));
    connect(m_actFilterByAuthors, SIGNAL(toggled(bool)), SLOT(filterByAuthors(bool)));
    connect(actionSelected, SIGNAL(triggered()), SLOT(showSelected()));
    connect(actionClearText, SIGNAL(triggered()), SIGNAL(clearText()));
//    connect(m_filterProxy, SIGNAL(layoutChanged()), SLOT(enableCatSelection()));
}

SearchFilterHandler::~SearchFilterHandler()
{
    delete m_menu;
}

void SearchFilterHandler::setShamelaModels(ShamelaModels *shaModel)
{
    m_shaModel = shaModel;
    m_filterProxy->setSourceModel(m_shaModel->booksModel());
}

void SearchFilterHandler::setFilterText(QString text)
{
    m_filterText = text;

    QRegExp rx("[0-9\\?\\*-]+:[0-9\\?\\*-]+");
    if(rx.indexIn(text) == -1) {
        text.replace(QRegExp("[\\x0627\\x0622\\x0623\\x0625]"), "[\\x0627\\x0622\\x0623\\x0625]");//ALEFs
        text.replace(QRegExp("[\\x0647\\x0629]"), "[\\x0647\\x0629]"); //TAH_MARBUTA, HEH
        //text.replace(QRegExp("[\\x062F\\x0630]"), "[\\x062F\\x0630]"); //DAL, THAL
        text.replace(QRegExp("[\\x064A\\x0649]"), "[\\x064A\\x0649]"); //YAH, ALEF MAKSOURA

        m_filterProxy->setFilterKeyColumn(m_actFilterByBooks->isChecked() ? 0 : 1);
        m_filterProxy->setFilterRole(Qt::DisplayRole);
        m_filterProxy->setFilterRegExp(text);
    } else {
        QStringList t = text.split(':');
        int dStart = (t.first() == "*") ? 0x80000000 : t.first().toInt();
        int dEnd = (t.last() == "*") ? 99999 : t.last().toInt();

        //qDebug("Authors from %d to %d", dStart, dEnd);
        m_filterProxy->setFilterByDeath(dStart, dEnd);
        m_filterProxy->setFilterRegExp("");
        m_filterProxy->setFilterKeyColumn(2);
    }
    if(m_hideFilterWidgetOnChange)
        m_selectedFilterWidget->hide();

    enableCatSelection();
}

ShamelaFilterProxyModel *SearchFilterHandler::getFilterModel()
{
    return m_filterProxy;
}

QMenu *SearchFilterHandler::getFilterLineMenu()
{
    return m_menu;
}

void SearchFilterHandler::filterByBooks(bool booksFilter)
{
    m_actFilterByBooks->blockSignals(true);
    m_actFilterByAuthors->blockSignals(true);

    if(booksFilter) {
        m_filterProxy->setFilterKeyColumn(0);
        m_actFilterByAuthors->setChecked(false);
    } else {
        m_filterProxy->setFilterKeyColumn(1);
        m_actFilterByAuthors->setChecked(true);
    }

    setFilterText("");

    m_actFilterByBooks->blockSignals(false);
    m_actFilterByAuthors->blockSignals(false);
}

void SearchFilterHandler::filterByAuthors(bool authorsFilter)
{
    m_actFilterByBooks->blockSignals(true);
    m_actFilterByAuthors->blockSignals(true);

    if(authorsFilter) {
        m_filterProxy->setFilterKeyColumn(1);
        m_actFilterByBooks->setChecked(false);
    } else {
        m_filterProxy->setFilterKeyColumn(0);
        m_actFilterByBooks->setChecked(true);
    }

    setFilterText("");

    m_actFilterByBooks->blockSignals(false);
    m_actFilterByAuthors->blockSignals(false);
}

void SearchFilterHandler::showSelected()
{
    QVariant checked(Qt::Checked);

    m_filterProxy->setFilterRole(Qt::CheckStateRole);
    m_filterProxy->setFilterFixedString(checked.toString());

    m_selectedFilterWidget->setText(tr("عرض ما تم اختياره"));
    m_selectedFilterWidget->show();
    m_hideFilterWidgetOnChange = false;
}

void SearchFilterHandler::showUnSelected()
{
    QVariant unChecked(Qt::Unchecked);

    m_filterProxy->setFilterRole(Qt::CheckStateRole);
    m_filterProxy->setFilterFixedString(unChecked.toString());

    m_selectedFilterWidget->setText(tr("عرض ما لم يتم اختياره"));
    m_selectedFilterWidget->show();
    m_hideFilterWidgetOnChange = false;
}

void SearchFilterHandler::setSelectedFilterWidget(SelectedFilterWidget *widget)
{
    m_selectedFilterWidget = widget;
    connect(m_selectedFilterWidget, SIGNAL(deleteFilter()), SLOT(clearFilter()));
    connect(m_selectedFilterWidget, SIGNAL(deleteFilter()),
            m_selectedFilterWidget, SLOT(hide()));
}

SelectedFilterWidget * SearchFilterHandler::selectedFilterWidget()
{
    return m_selectedFilterWidget;
}

void SearchFilterHandler::clearFilter()
{
    m_filterProxy->setFilterRole(Qt::DisplayRole);
    m_filterProxy->setFilterRegExp("");
}

void SearchFilterHandler::enableCatSelection()
{
    bool catCheckable = m_filterProxy->filterRegExp().isEmpty();

    int rowCount = m_filterProxy->rowCount();
    QModelIndex topLeft = m_filterProxy->index(0, 0);
    QModelIndex bottomRight = m_filterProxy->index(rowCount-1, 0);
    QItemSelection selection(topLeft, bottomRight);

    foreach (QModelIndex index, selection.indexes()) {
        m_shaModel->booksModel()->item(index.row())->setCheckable(catCheckable);
    }
}
