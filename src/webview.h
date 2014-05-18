#ifndef KWEBVIEW_H
#define KWEBVIEW_H

#include <qwebview.h>
#include <qwebframe.h>
#include "shamelaindexinfo.h"

class WebView : public QWebView
{
    Q_OBJECT
public:
    WebView(ShamelaIndexInfo::IndexType indexType, QWidget* parent = 0);
    void execJS(QString js);
    void addObject(const QString &name, QObject *object);
    void setIndexType(ShamelaIndexInfo::IndexType indexType);
    void init();
    QString html();

public slots:
    void setText(const QString &text);

protected:
    QWebFrame *m_frame;
    ShamelaIndexInfo::IndexType m_indexType;
    QString m_html;
};

#endif // KWEBVIEW_H
