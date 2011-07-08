#include "quranresult.h"

QuranResult::QuranResult()
{
    m_edited = true;
}

QuranResult::~QuranResult()
{
}

QString QuranResult::text()
{
    return m_text;
}

QString QuranResult::soraName()
{
    return m_soraName;
}

QString QuranResult::snippet()
{
    return m_snippet;
}

QString QuranResult::bgColor()
{
    return m_bgColor;
}

QString QuranResult::toHtml()
{
    if(m_edited) {
        m_html = QObject::tr("<div class=\"result %1\">"
                                 "<div class=\"result_head\">"
                                 "<h3>سورة %2، الاية: %8</h3>"
                                 "<span class=\"progSpan\" style=\"width: %3px;\">"
                                 "<span class=\"progSpanContainre\"></span>"
                                 "</span>"
                                 "</div>"
                                 "<p class=\"resultText\" bookid=\"%4\" rid=\"%5\" >%6</p>"
                                 "<p class=\"result_info\"><span class=\"bookName\">&nbsp;</span>"
                                 "<span style=\"float: left\">"
                                 "<span style=\"margin: 5px 0px\">رقم السورة: <span style=\"margin-left: 7px;\">%4</span></span>"
                                 "<span style=\"margin: 5px 0px\">الاية: <span style=\"margin-left: 7px;\">%8</span></span>"
                                 "<span style=\"margin: 5px 0px\">الصفحة: <span style=\"margin-left: 7px;\">%7</span></span>"
                                 "</span></p></div>")
                .arg(m_bgColor)                 // backround class name (%1)
                .arg(m_soraName)                // Sora name            (%2)
                .arg(m_score)                   // score                (%3)
                .arg(m_soraNumber)              // Sora number          (%4)
                .arg(m_id)                      // Result id            (%5)
                .arg(m_snippet.simplified())    // text snippet         (%6)
                .arg(m_page)                    // page                 (%7)
                .arg(m_aya);                    // aya                  (%8)


        m_edited = false;
    }

    return m_html;
}

int QuranResult::page()
{
    return m_page;
}

int QuranResult::part()
{
    return m_part;
}

int QuranResult::pageID()
{
    return m_pageID;
}

int QuranResult::score()
{
    return m_score;
}

int QuranResult::id()
{
    return m_id;
}

int QuranResult::aya()
{
    return m_aya;
}

int QuranResult::soraNumber()
{
    return m_soraNumber;
}

void QuranResult::setText(const QString &text)
{

    if(m_text != text) {
        m_text = text;
        clearShorts(m_text);

        m_edited = true;
    }
}

void QuranResult::setSnippet(const QString &snippet)
{
    if(m_snippet != snippet) {
        m_snippet = snippet;
        clearShorts(m_snippet);

        m_edited = true;
    }
}

void QuranResult::setBgColor(const QString &color)
{

    if(m_bgColor != color) {
        m_bgColor = color;
        m_edited = true;

    }
}

void QuranResult::setSoraName(const QString &name)
{
    if(name != m_soraName) {
        m_soraName = name;
        m_edited = true;
    }
}

void QuranResult::setPage(int page)
{

    if(m_page != page) {
        m_page = page;
        m_edited = true;
    }
}

void QuranResult::setPart(int part)
{
    if(m_part != part){
        m_part = part;
        m_edited = true;
    }
}

void QuranResult::setPageId(int id)
{
    if(m_pageID != id) {
        m_pageID = id;
        m_edited = true;
    }
}

void QuranResult::setBookId(int id)
{
    if(m_bookId != id) {
        m_bookId = id;
        m_edited = true;
    }
}

void QuranResult::setScore(int score)
{
    if(m_score != score) {
        m_score = score;
        m_edited = true;
    }
}

void QuranResult::setId(int id)
{
    if(m_id != id) {
        m_id = id;
        m_edited = true;
    }
}

void QuranResult::setAya(int num)
{
    if(m_aya != num) {
        m_aya = num;
        m_edited = true;
    }
}

void QuranResult::setSoraNumber(int num)
{
    if(m_soraNumber != num) {
        m_soraNumber = num;
        m_edited = true;
    }
}
