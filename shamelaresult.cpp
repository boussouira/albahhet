#include "shamelaresult.h"
#include "common.h"

ShamelaResult::ShamelaResult()
{
    m_edited = true;
}

ShamelaResult::~ShamelaResult()
{
}

QString ShamelaResult::text()
{
    return m_text;
}

QString ShamelaResult::title()
{
    return m_title;
}

QString ShamelaResult::snippet()
{
    return m_snippet;
}

QString ShamelaResult::bgColor()
{
    return m_bgColor;
}

QString ShamelaResult::toHtml()
{
    if(m_edited) {
        m_html = QObject::trUtf8("<div class=\"result %1\">"
                                 "<div class=\"result_head\">"
                                 "<h3>%2</h3>"
                                 "<span class=\"progSpan\" style=\"width: %10px;\">"
                                 "<span class=\"progSpanContainre\"></span>"
                                 "</span>"
                                 "</div>"
                                 "<a class=\"bookLink\" href=\"http://localhost/book.html?id=%3&bookid=%8&archive=%9\">%4</a>"
                                 "<p class=\"result_info\"> كتاب: <span class=\"bookName\">%5</span>"
                                 "<span style=\"float: left;margin: 5px 0px\">الصفحة: <span style=\"margin-left: 7px;\">%6</span>  الجزء: <span>%7</span></span>"
                                 "</p></div>")
                .arg(m_bgColor)     // backround class name
                .arg(m_title)       // bab
                .arg(m_id)          // entry id
                .arg(m_snippet.simplified())    // text snippet
                .arg(m_bookName)     // book name
                .arg(m_page)        // page
                .arg(m_part)        // part
                .arg(m_bookId)      // book id
                .arg(m_archive)     // book archive
                .arg(m_score);      // score

        m_edited = false;
    }

    return m_html;
}

int ShamelaResult::page()
{
    return m_page;
}

int ShamelaResult::part()
{
    return m_part;
}

int ShamelaResult::id()
{
    return m_id;
}

int ShamelaResult::bookId()
{
    return m_bookId;
}

int ShamelaResult::archive()
{
    return m_archive;
}

int ShamelaResult::score()
{
    return m_score;
}

void ShamelaResult::setText(const QString &text)
{

    if(m_text != text) {
        m_text = text;
        m_edited = true;
    }
}

void ShamelaResult::setTitle(const QString &title)
{

    if(m_title != title) {
        m_title = title;
        m_edited = true;
    }
}

void ShamelaResult::setSnippet(const QString &snippet)
{
    if(m_snippet != snippet) {
        m_snippet = snippet;
        m_edited = true;
    }
}

void ShamelaResult::setBgColor(const QString &color)
{

    if(m_bgColor != color) {
        m_bgColor = color;
        m_edited = true;

    }
}

void ShamelaResult::setBookName(const QString &name)
{
    if(name != m_bookName) {
        m_bookName = name;
        m_edited = true;
    }
}

void ShamelaResult::setPage(int page)
{

    if(m_page != page) {
        m_page = page;
        m_edited = true;
    }
}

void ShamelaResult::setPart(int part)
{
    if(m_part != part){
        m_part = part;
        m_edited = true;
    }
}

void ShamelaResult::setId(int id)
{
    if(m_id != id) {
        m_id = id;
        m_edited = true;
    }
}

void ShamelaResult::setBookId(int id)
{
    if(m_bookId != id) {
        m_bookId = id;
        m_edited = true;
    }
}

void ShamelaResult::setArchive(int archive)
{
    if(m_archive != archive) {
        m_archive = archive;
        m_edited = true;
    }
}

void ShamelaResult::setScore(int score)
{
    if(m_score != score) {
        m_score = score;
        m_edited = true;
    }
}
