#include "bookprogresswidget.h"
#include "shamelaindexer.h"
#include <qboxlayout.h>
#include <qheaderview.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtoolbutton.h>
#include <qgroupbox.h>

BookProgressWidget::BookProgressWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;

    verticalLayout_2 = new QVBoxLayout(this);
    verticalLayout_2->setSpacing(0);
    verticalLayout_2->setContentsMargins(0, 0, 0, 0);

    groupBox = new QGroupBox(this);
    verticalLayout = new QVBoxLayout(groupBox);
    m_label = new QLabel(groupBox);

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_label->sizePolicy().hasHeightForWidth());

    m_label->setSizePolicy(sizePolicy);

    verticalLayout->addWidget(m_label);
    verticalLayout_2->addWidget(groupBox);

}

void BookProgressWidget::setName(QString name)
{
    m_label->setText(name);
}
