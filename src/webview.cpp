#include "webview.h"
#include "common.h"
#include <qapplication.h>
#include <qsettings.h>
#include <qaction.h>

WebView::WebView(IndexInfo::IndexType indexType, QWidget *parent) : QWebView(parent), m_indexType(indexType)
{
    m_frame = page()->mainFrame();

    QSettings settings;

    settings.beginGroup("BooksViewer");
    QString fontString = settings.value("fontFamily", QWebSettings::globalSettings()->fontFamily(QWebSettings::StandardFont)).toString();
    int fontSize = settings.value("fontSize", QWebSettings::globalSettings()->fontSize(QWebSettings::DefaultFontSize)).toInt();
    settings.endGroup();

    QFont font;
    font.fromString(fontString);

    pageAction(QWebPage::Copy)->setShortcut(QKeySequence::Copy);

    //QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setFontFamily(QWebSettings::StandardFont, font.family());
    QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFontSize, fontSize);
}

void WebView::setIndexType(IndexInfo::IndexType indexType)
{
    m_indexType = indexType;
}

void WebView::init()
{
    QString appPath = QUrl::fromLocalFile(qApp->applicationDirPath()).toString();
    QString jsFile;

    if(m_indexType == IndexInfo::ShamelaIndex)
        jsFile = "shamela.js";
    else
        qFatal("Unknow index type");

    m_html = QString("<html><head><title></title>"
                     "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>"
                     "<link href=\"%1/data/default.css\" rel=\"stylesheet\" type=\"text/css\"/>"
                     "</head>"
                     "<body><div id=\"searchResult\"></div>"
                     "<script type=\"text/javascript\" src=\"%1/data/jquery-1.4.2.min.js\"></script>"
                     "<script type=\"text/javascript\" src=\"%1/data/jquery.dimensions.js\"></script>"
                     "<script type=\"text/javascript\" src=\"%1/data/jquery.tooltip.js\"></script>"
                     "<script type=\"text/javascript\" src=\"%1/data/%2\"></script>"
                     "</body></html>")
             .arg(appPath)
             .arg(jsFile);

    setHtml(m_html);
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
