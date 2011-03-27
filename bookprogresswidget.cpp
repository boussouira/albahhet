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
    QHBoxLayout *horizontalLayout;

    verticalLayout_2 = new QVBoxLayout(this);
    verticalLayout_2->setSpacing(0);
    verticalLayout_2->setContentsMargins(0, 0, 0, 0);

    groupBox = new QGroupBox(this);
//    groupBox->setFlat(true);
    verticalLayout = new QVBoxLayout(groupBox);
    m_label = new QLabel(groupBox);

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_label->sizePolicy().hasHeightForWidth());

    m_label->setSizePolicy(sizePolicy);
    verticalLayout->addWidget(m_label);

    horizontalLayout = new QHBoxLayout();
    m_progressBar = new QProgressBar(groupBox);

    QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(m_progressBar->sizePolicy().hasHeightForWidth());

    m_progressBar->setSizePolicy(sizePolicy1);
    m_progressBar->setAlignment(Qt::AlignCenter);
    horizontalLayout->addWidget(m_progressBar);

    m_buttonStop = new QToolButton(groupBox);
    m_buttonStop->setIcon(QIcon(":/bin/data/images/delete.png"));
    m_buttonStop->setAutoRaise(true);
    horizontalLayout->addWidget(m_buttonStop);

    verticalLayout->addLayout(horizontalLayout);
    verticalLayout_2->addWidget(groupBox);

    connect(m_buttonStop, SIGNAL(clicked()), SIGNAL(skipCurrent()));
}

void BookProgressWidget::setMax(int max)
{
    m_progressBar->setMaximum(max);
}

void BookProgressWidget::setName(QString name)
{
    m_label->setText(name);
    m_progressBar->setValue(0);
}

void BookProgressWidget::setCurrentValue(int value)
{
    m_progressBar->setValue(value);
}

void BookProgressWidget::reset()
{
    m_progressBar->setValue(0);
    m_label->clear();
}
