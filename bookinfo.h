#ifndef BOOKINFO_H
#define BOOKINFO_H

#include <qstring.h>
#include <tchar.h>
#include <qdebug.h>

class IndexInfo;

class BookInfo
{
public:
    BookInfo();
    BookInfo(int id, QString name, QString path, int archive);
    ~BookInfo();

    int id() { return m_id; }
    int archive() { return m_archive; }
    int cat() { return m_cat; }
    int authorID() { return m_authorID; }
    QString name() { return m_name; }
    QString path() { return m_path; }
    QString mainTable() { return m_mainTable; }
    QString tocTable() { return m_tocTable; }

    void genInfo();
    void genInfo(IndexInfo *info);

    TCHAR *idT() { return m_idT; }
    TCHAR *archiveT() { return m_archiveT; }
    TCHAR *authorIDT() { return m_authorIDT; }
    TCHAR *catT() { return m_catT; }

    void setId(int id);
    void setArchive(int archive);
    void setCat(int cat);
    void setAuthorID(int id);
    void setName(const QString &name);
    void setPath(const QString &path);
    void debug();

protected:
    void init();

protected:
    int m_id;
    QString m_name;
    QString m_path;
    QString m_mainTable;
    QString m_tocTable;
    int m_archive;
    int m_cat;
    int m_authorID;
    TCHAR *m_idT;
    TCHAR *m_archiveT;
    TCHAR *m_authorIDT;
    TCHAR *m_catT;
};

#endif // BOOKINFO_H
