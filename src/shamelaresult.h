#ifndef SHAMELARESULT_H
#define SHAMELARESULT_H

#include <qstring.h>

class ShamelaResult
{
public:
    ShamelaResult();
    ~ShamelaResult();

    int id();
    QString text();
    QString title();
    QString snippet();
    QString bgColor();
    QString bookInfo();
    QString toHtml();
    int page();
    int part();
    int pageID();
    int bookId();
    int archive();
    int score();

    void setId(int id);
    void setText(const QString &text);
    void setTitle(const QString &title);
    void setSnippet(const QString &snippet);
    void setBgColor(const QString &color);
    void setBookName(const QString &name);
    void setBookInfo(const QString &info);
    void setPage(int page);
    void setPart(int part);
    void setPageId(int pageID);
    void setBookId(int pageID);
    void setArchive(int archive);
    void setScore(int score);

protected:
    int m_id;
    QString m_text;
    QString m_title;
    QString m_snippet;
    QString m_bgColor;
    QString m_bookName;
    QString m_info;
    QString m_html;
    bool m_edited;
    int m_page;
    int m_part;
    int m_pageID;
    int m_bookId;
    int m_archive;
    int m_score;
};

#endif // SHAMELARESULT_H
