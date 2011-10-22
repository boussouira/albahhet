#include "webview.h"
#include "common.h"
#include <qapplication.h>
#include <qsettings.h>

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

    QWebSettings::globalSettings()->setFontFamily(QWebSettings::StandardFont, font.family());
    QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFontSize, fontSize);
}

void WebView::setIndexType(IndexInfo::IndexType indexType)
{
    m_indexType = indexType;
}

void WebView::init()
{
    QString appPath(QString("file:///%1").arg(qApp->applicationDirPath()));
    QString jsFile;

    if(m_indexType == IndexInfo::ShamelaIndex)
        jsFile = "shamela.js";
    else if(m_indexType == IndexInfo::QuranIndex)
        jsFile = "quran.js";
    else
        qFatal("Unknow index type");

    setHtml(QString("<html><head><title></title>"
                    "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>"
                    "<link href=\"%1/data/default.css\" rel=\"stylesheet\" type=\"text/css\"/>"
                    "</head>"
                    "<body></body>"
                    "<script type=\"text/javascript\" src=\"%1/data/jquery-1.4.2.min.js\" />"
                    "<script type=\"text/javascript\" src=\"%1/data/jquery.dimensions.js\" />"
                    "<script type=\"text/javascript\" src=\"%1/data/jquery.tooltip.js\" />"
                    "<script type=\"text/javascript\" src=\"%1/data/%2\" />"
                    "</html>")
            .arg(appPath)
            .arg(jsFile));
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
