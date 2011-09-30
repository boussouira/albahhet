#ifndef SEARCHFILTERHANDLER_H
#define SEARCHFILTERHANDLER_H

#include <QObject>

class ShamelaModels;
class ShamelaFilterProxyModel;
class SelectedFilterWidget;
class QMenu;
class QAction;

class SearchFilterHandler : public QObject
{
    Q_OBJECT
public:
    SearchFilterHandler(QObject *parent = 0);
    ~SearchFilterHandler();
    void setShamelaModels(ShamelaModels *shaModel);
    void setSelectedFilterWidget(SelectedFilterWidget *widget);
    ShamelaFilterProxyModel *getFilterModel();
    QMenu *getFilterLineMenu();
    SelectedFilterWidget *selectedFilterWidget();

public slots:
    void setFilterText(QString text);
    void filterByBooks(bool booksFilter);
    void filterByAuthors(bool authorsFilter);
    void showSelected();
    void showUnSelected();
    void clearFilter();
    void enableCatSelection();

signals:
    void clearText();

protected:
    ShamelaModels *m_shaModel;
    ShamelaFilterProxyModel *m_filterProxy;
    SelectedFilterWidget *m_selectedFilterWidget;
    QMenu *m_menu;
    QAction *m_actFilterByBooks;
    QAction *m_actFilterByAuthors;
    QString m_filterText;
    int m_index;
    bool m_hideFilterWidgetOnChange;
};

#endif // SEARCHFILTERHANDLER_H
