#ifndef SELECTEDFILTERWIDGET_H
#define SELECTEDFILTERWIDGET_H

#include <QWidget>

namespace Ui {
    class SelectedFilterWidget;
}

class SelectedFilterWidget : public QWidget
{
    Q_OBJECT

public:
    SelectedFilterWidget(QWidget *parent = 0);
    ~SelectedFilterWidget();
    void setText(QString text);

signals:
    void deleteFilter();

private:
    Ui::SelectedFilterWidget *ui;
};

#endif // SELECTEDFILTERWIDGET_H
