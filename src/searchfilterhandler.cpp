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
    m_actFilterByBetaka = menu2->addAction(tr("بطاقة الكتاب"));
    m_actFilterByAuthors = menu2->addAction(tr("اسماء المؤلفين"));

    m_actFilterByBooks->setCheckable(true);
    m_actFilterByBooks->setChecked(true);
    m_actFilterByBetaka->setCheckable(true);
    m_actFilterByAuthors->setCheckable(true);

    m_hideFilterWidgetOnChange = false;
    m_filterColumn = 0;
    m_role = Qt::DisplayRole;

    connect(m_actFilterByBooks, SIGNAL(triggered()), SLOT(changeFilterAction()));
    connect(m_actFilterByBetaka, SIGNAL(triggered()), SLOT(changeFilterAction()));
    connect(m_actFilterByAuthors, SIGNAL(triggered()), SLOT(changeFilterAction()));
    connect(actionSelected, SIGNAL(triggered()), SLOT(showSelected()));
    connect(actionClearText, SIGNAL(triggered()), SIGNAL(clearText()));
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
        text.replace(QRegExp("[\\x064A\\x0649]"), "[\\x064A\\x0649]"); //YAH, ALEF MAKSOURA

        m_filterProxy->setFilterKeyColumn(m_filterColumn);
        m_filterProxy->setFilterRole(m_role);
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

void SearchFilterHandler::changeFilterAction()
{
    QAction *act = qobject_cast<QAction*>(sender());
    QList<QAction*> actList;

    if(act) {
        if(!act->isChecked()) {
            act->setChecked(true);
            return;
        }

        actList << m_actFilterByBooks;
        actList << m_actFilterByBetaka;
        actList << m_actFilterByAuthors;

        foreach (QAction *a, actList) {
            if(act != a)
                a->setChecked(false);
        }

        if(act == m_actFilterByBooks) {
            m_filterColumn = 0;
            m_role = Qt::DisplayRole;
        } else if(act == m_actFilterByAuthors) {
            m_filterColumn = 1;
            m_role = Qt::DisplayRole;
        } else if(act == m_actFilterByBetaka) {
            m_filterColumn = 0;
            m_role = Qt::ToolTipRole;
        }

        m_filterProxy->setFilterKeyColumn(m_filterColumn);
        m_filterProxy->setFilterRole(m_role);
    }
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
