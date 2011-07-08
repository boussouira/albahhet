#ifndef KWEBVIEW_H
#define KWEBVIEW_H

#include <qwebview.h>
#include <qwebframe.h>
#include "indexinfo.h"

class WebView : public QWebView
{
    Q_OBJECT
public:
    WebView(IndexInfo::IndexType indexType, QWidget* parent = 0);
    void execJS(QString js);
    void addObject(const QString &name, QObject *object);
    void setIndexType(IndexInfo::IndexType indexType);
    void init();
    QString html();

public slots:
    void setText(const QString &text);

protected:
    QWebFrame *m_frame;
    IndexInfo::IndexType m_indexType;
};

#endif // KWEBVIEW_H
