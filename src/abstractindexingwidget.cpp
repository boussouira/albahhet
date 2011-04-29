#include "abstractindexingwidget.h"
#include "common.h"
#include "cl_common.h"
#include "bookinfo.h"
#include "indexesmanager.h"
#include <QMessageBox>
#include <QPushButton>

AbstractIndexingWidget::AbstractIndexingWidget(QWidget *parent) : QWidget(parent)
{
    m_bookDB = new BooksDB();
}

void AbstractIndexingWidget::setIndexManager(IndexesManager *manager)
{
    m_indexesManager = manager;
}

void AbstractIndexingWidget::setButtons(QPushButton *nextButton, QPushButton *cancelButton)
{
    m_nextButton = nextButton;
    m_cancelButton = cancelButton;
}

void AbstractIndexingWidget::shutDown()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("انتهت عملية الفهرسة بنجاح" "<br>"
                      "سيتم اطفاء الجهاز بعد %1")
                   .arg(arPlural(10,  SECOND, true)));
    msgBox.setStandardButtons(QMessageBox::Cancel);

    QTimeLine timeLine(10000, this);
    timeLine.setUpdateInterval(1000);
    timeLine.setDirection(QTimeLine::Backward);

    connect(&msgBox, SIGNAL(buttonClicked(QAbstractButton*)), &msgBox, SLOT(reject()));
    connect(&timeLine, SIGNAL(finished()), &msgBox, SLOT(accept()));
    connect(&timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(shutDownUpdateTime(qreal)));

    m_shutDownTime = 10;
    m_shutDownMsgBox = &msgBox;

    timeLine.start();
    int ret = msgBox.exec();

    if(ret) {
#ifdef Q_OS_WIN
        system("shutdown -s -t 00");
#endif
    }
}

void AbstractIndexingWidget::shutDownUpdateTime(qreal)
{
    m_shutDownMsgBox->setText(tr("انتهت عمليو الفهرسة بنجاح" "<br>"
                                 "سيتم اطفاء الجهاز بعد %1")
                              .arg(arPlural(m_shutDownTime--,  SECOND, true)));
}
