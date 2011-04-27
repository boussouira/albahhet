#ifndef SEARCHFILTERHANDLER_H
#define SEARCHFILTERHANDLER_H

#include <QObject>

class ShamelaModels;
class ShamelaFilterProxyModel;
class SelectedFilterWidget;
class QMenu;

class SearchFilterHandler : public QObject
{
    Q_OBJECT
public:
    SearchFilterHandler(QObject *parent = 0);
    ~SearchFilterHandler();
    void setShamelaModels(ShamelaModels *shaModel);
    void setFilterModel(ShamelaFilterProxyModel *filter);
    void setSelectedFilterWidget(SelectedFilterWidget *widget);
    ShamelaFilterProxyModel *getFilterModel();
    QMenu *getFilterLineMenu();
    SelectedFilterWidget *selectedFilterWidget();
    void setClearFilterOnChange(bool clear);
    bool clearFilterOnChange();

public slots:
    void setFilterText(QString text);
    void setFilterSourceModel(int index);
    void chooseFilterSourceModel();
    void ignoreSameChars(bool ignore);
    void showSelected();
    void showUnSelected();
    void clearFilter();

protected:
    ShamelaModels *m_shaModel;
    ShamelaFilterProxyModel *m_filterProxy;
    SelectedFilterWidget *m_selectedFilterWidget;
    QMenu *m_menu;
    QString m_filterText;
    int m_index;
    bool m_ignore;
    bool m_clearFilterOnChange;
};

#endif // SEARCHFILTERHANDLER_H
