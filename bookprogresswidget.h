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
    void setMax(int max);
    void setName(QString name);
    void setCurrentValue(int value);
    void reset();

signals:
    void skipCurrent();

protected:
    QToolButton *m_buttonStop;
    QProgressBar *m_progressBar;
    QLabel *m_label;
};

#endif // BOOKPROGRESSWIDGET_H
