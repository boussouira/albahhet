#ifndef BOOKPROGRESSWIDGET_H
#define BOOKPROGRESSWIDGET_H

#include <qwidget.h>

class QHBoxLayout;
class QToolButton;
class QProgressBar;
class QLabel;
class ShamelaIndexer;

class BookProgressWidget : public QWidget
{
    Q_OBJECT

public:
    BookProgressWidget(QWidget *parent=0);

public slots:
    void setName(QString name);

protected:
    QToolButton *m_buttonStop;
    QProgressBar *m_progressBar;
    QLabel *m_label;
};

#endif // BOOKPROGRESSWIDGET_H
