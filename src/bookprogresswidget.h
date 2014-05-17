#ifndef BOOKPROGRESSWIDGET_H
#define BOOKPROGRESSWIDGET_H

#include <qtimer.h>
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
    BookProgressWidget(bool progressBar=false, QWidget *parent=0);

public slots:
    void setName(QString name);
    void setTotalProgress(int total);
    void setProgress(int progress);

protected slots:
    void progressInterval();

protected:
    QToolButton *m_buttonStop;
    QProgressBar *m_progressBar;
    QLabel *m_label;
    QTimer m_timer;
    bool m_showProgress;
    bool m_changeLabelHeight;
};

#endif // BOOKPROGRESSWIDGET_H
