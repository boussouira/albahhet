#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <qtabwidget.h>
#include <qtabbar.h>

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    TabWidget(QWidget *parent = 0);
    ~TabWidget();

public slots:
    void updateTabBar();
    void closeTab(int index);

signals:
    void tabMoved(int from, int to);

private:
    QTabBar *m_tabBar;

};

#endif // TABWIDGET_H
