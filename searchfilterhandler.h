#ifndef SEARCHFILTERHANDLER_H
#define SEARCHFILTERHANDLER_H

#include <QObject>

class ShamelaModels;
class QSortFilterProxyModel;
class QMenu;

class SearchFilterHandler : public QObject
{
    Q_OBJECT
public:
    SearchFilterHandler(QObject *parent = 0);
    ~SearchFilterHandler();
    void setShamelaModels(ShamelaModels *shaModel);
    void setFilterModel(QSortFilterProxyModel *filter);
    QSortFilterProxyModel *getFilterModel();
    QMenu *getFilterLineMenu();

public slots:
    void setFilterText(QString text);
    void setFilterSourceModel(int index);
    void chooseFilterSourceModel();
    void ignoreSameChars(bool ignore);
    void showSelected();
    void showUnSelected();

protected:
    ShamelaModels *m_shaModel;
    QSortFilterProxyModel *m_filterProxy;
    QMenu *m_menu;
    QString m_filterText;
    int m_index;
    bool m_ignore;
};

#endif // SEARCHFILTERHANDLER_H
