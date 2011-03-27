#include "webview.h"
#include "common.h"
#include <qapplication.h>
#include <qsettings.h>

WebView::WebView(QWidget *parent) : QWebView(parent)
{
    m_frame = page()->mainFrame();

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);

    settings.beginGroup("BooksViewer");
    QString fontString = settings.value("fontFamily", QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont)).toString();
    int fontSize = settings.value("fontSize", QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize)).toInt();
    settings.endGroup();

    QFont font;
    font.fromString(fontString);

    QWebSettings::globalSettings()->setFontFamily(QWebSettings::StandardFont, font.family());
    QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFontSize, fontSize);
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
