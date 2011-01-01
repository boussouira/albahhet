#include "shamelaresult.h"

ShamelaResult::ShamelaResult()
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
    m_text = text;
}

void ShamelaResult::setTitle(const QString &title)
{
    m_title = title;
}

void ShamelaResult::setSnippet(const QString &text)
{
    m_snippet = text;
}

void ShamelaResult::setBgColor(const QString &color)
{
    m_bgColor = color;
}

void ShamelaResult::setPage(int page)
{
    m_page = page;
}

void ShamelaResult::setPart(int part)
{
    m_part = part;
}

void ShamelaResult::setId(int id)
{
    m_id = id;
}

void ShamelaResult::setBookId(int id)
{
    m_bookId = id;
}

void ShamelaResult::setArchive(int archive)
{
    m_archive = archive;
}

void ShamelaResult::setScore(int score)
{
    m_score = score;
}
