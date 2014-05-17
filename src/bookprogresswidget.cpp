#include "bookprogresswidget.h"
#include "shamelaindexer.h"
#include <qboxlayout.h>
#include <qheaderview.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtoolbutton.h>
#include <qgroupbox.h>

BookProgressWidget::BookProgressWidget(bool progressBar, QWidget *parent) :
    QWidget(parent),
    m_showProgress(progressBar),
    m_changeLabelHeight(true)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox *groupBox = new QGroupBox(this);

    m_label = new QLabel(groupBox);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setAlignment(Qt::AlignCenter);
    m_progressBar->setTextVisible(false);
    m_progressBar->setVisible(false);
    m_progressBar->setMinimumWidth(100);
    m_progressBar->setMaximumWidth(100);

    QHBoxLayout *boxLayout = new QHBoxLayout(groupBox);
    boxLayout->addWidget(m_label);
    boxLayout->addStretch();
    boxLayout->addWidget(m_progressBar);

    layout->addWidget(groupBox);

    if(progressBar) {
        m_timer.setInterval(5000);
        m_timer.setSingleShot(true);
        connect(&m_timer, SIGNAL(timeout()), SLOT(progressInterval()));
    }
}

void BookProgressWidget::setName(QString name)
{
    m_label->setText(name);

    if(m_showProgress) {
        m_progressBar->setValue(0);
        m_progressBar->hide();

        m_timer.start();
    }
}

void BookProgressWidget::setTotalProgress(int total)
{
    if(m_showProgress) {
        m_progressBar->setValue(0);
        m_progressBar->setMaximum(total);
    }
}

void BookProgressWidget::setProgress(int progress)
{
    if(m_showProgress) {
        m_progressBar->setValue(progress);
    }
}

void BookProgressWidget::progressInterval()
{
    if(m_showProgress) {
        m_progressBar->show();

        if(m_changeLabelHeight) {
            m_label->setMinimumHeight(m_progressBar->height());
            m_label->setMaximumHeight(m_progressBar->height());

            m_changeLabelHeight = false;
        }
    }
}
