#include "webview.h"

WebView::WebView(QWidget *parent) : QWebView(parent)
{
    m_frame = page()->mainFrame();
}

void WebView::setText(const QString &text)
{
    setHtml(text);
}

void WebView::execJS(QString js)
{
    m_frame->evaluateJavaScript(js);
}

QString WebView::html()
{
    return m_frame->toHtml();
}

void WebView::addObject(const QString &name, QObject *object)
{
    m_frame->addToJavaScriptWindowObject(name, object);
}
