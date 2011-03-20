#include "tabwidget.h"

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent)
{
    m_tabBar = new QTabBar(this);

    m_tabBar->setTabsClosable(true);
    setTabBar(m_tabBar);

    connect(m_tabBar, SIGNAL(tabMoved(int, int)), this, SIGNAL(tabMoved(int,int)));
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(updateTabBar()));
    connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

void TabWidget::updateTabBar()
{
    if(m_tabBar->count() > 1)
        m_tabBar->setTabsClosable(true);
    else
        m_tabBar->setTabsClosable(false);
}

void TabWidget::closeTab(int index)
{
    if(index > 0) {
        QWidget *w = widget(index);

        if(w) {
            removeTab(index);
            delete w;
        }
    }

    updateTabBar();
}
