#ifndef KWEBVIEW_H
#define KWEBVIEW_H

#include <qwebview.h>
#include <qwebframe.h>

class WebView : public QWebView
{
    Q_OBJECT
public:
    WebView(QWidget* parent = 0);
    void execJS(QString js);
    QString html();

public slots:
    void setText(const QString &text);

protected:
    QWebFrame *m_frame;
};

#endif // KWEBVIEW_H
