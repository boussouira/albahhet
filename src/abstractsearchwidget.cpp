#include "abstractsearchwidget.h"

AbstractSearchWidget::AbstractSearchWidget(QWidget *parent) :
    QWidget(parent)
{
}

AbstractSearchWidget::~AbstractSearchWidget()
{
}

QList<int> AbstractSearchWidget::selectedBooks()
{
    QList<int> list;
    return list;
}

void AbstractSearchWidget::selectBooks(QList<int> books)
{
    Q_UNUSED(books);
}
