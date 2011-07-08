#ifndef QURANRESULT_H
#define QURANRESULT_H

#include "common.h"

class QuranResult
{
public:
    QuranResult();
    ~QuranResult();

    int id();
    QString text();
    QString soraName();
    QString snippet();
    QString bgColor();
    QString toHtml();
    int page();
    int part();
    int pageID();
    int score();
    int aya();
    int soraNumber();

    void setId(int id);
    void setText(const QString &text);
    void setSoraName(const QString &title);
    void setSnippet(const QString &snippet);
    void setBgColor(const QString &color);
    void setPage(int page);
    void setPart(int part);
    void setPageId(int pageID);
    void setBookId(int pageID);
    void setScore(int score);
    void setAya(int num);
    void setSoraNumber(int num);

protected:
    int m_id;
    QString m_text;
    QString m_soraName;
    QString m_snippet;
    QString m_bgColor;
    QString m_html;
    bool m_edited;
    int m_page;
    int m_part;
    int m_aya;
    int m_pageID;
    int m_bookId;
    int m_score;
    int m_soraNumber;
};

#endif // QURANRESULT_H
