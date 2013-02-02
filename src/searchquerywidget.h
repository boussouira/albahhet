#ifndef SEARCHQUERYWIDGET_H
#define SEARCHQUERYWIDGET_H

#include <qwidget.h>

namespace Ui {
class SearchQueryWidget;
}

namespace lucene {
namespace search {
class Query;
}
namespace queryParser {
class QueryParser;
}
}

class QStringListModel;
class QCompleter;

class SearchQueryWidget : public QWidget
{
    Q_OBJECT

public:
    SearchQueryWidget(QWidget *parent = 0);
    ~SearchQueryWidget();

    lucene::search::Query *searchQuery(lucene::queryParser::QueryParser *queryPareser);

    void setSearchQuery(const QString &text);

    void loadSettings();
    void saveSettings();

    void loadSearchQuery();
    void saveSearchQuery();

    QString searchQueryStr(bool clean=false);

protected:
    void updateSearchWidget();
    lucene::search::Query *defaultQuery(lucene::queryParser::QueryParser *queryPareser);
    lucene::search::Query *advancedQuery(lucene::queryParser::QueryParser *queryPareser);

protected slots:
    void toggoleAdvancedSearchWidget();
    void setupCleanMenu();
    void clearSpecialChar();
    void matchSearch();
    void removeTashekil();

signals:
    void search();

private:
    Ui::SearchQueryWidget *ui;
    QStringListModel *m_completerModel;
    QCompleter *m_completer;
    bool m_advancedSearch;
    QString m_searchQuery;
};

#endif // SEARCHQUERYWIDGET_H
