#include "selectedfilterwidget.h"
#include "ui_selectedfilterwidget.h"

SelectedFilterWidget::SelectedFilterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SelectedFilterWidget)
{
    ui->setupUi(this);

    connect(ui->toolButton, SIGNAL(clicked()), SIGNAL(deleteFilter()));
}

SelectedFilterWidget::~SelectedFilterWidget()
{
    delete ui;
}

void SelectedFilterWidget::setText(QString text)
{
    ui->label->setText(text);
}
