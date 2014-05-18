#ifndef BOOKINFO_H
#define BOOKINFO_H

#include <qstring.h>
#include <tchar.h>
#include <qdebug.h>

class ShamelaIndexInfo;

class BookInfo
{
public:
    BookInfo();
    BookInfo(int bid, QString bname, QString bpath, int barchive);
    ~BookInfo();

    int id() { return m_id; }
    int archive() { return m_archive; }
    int cat() { return m_cat; }
    int authorID() { return m_authorID; }
    int bookVersion() { return m_bookVersion; }
    QString name() { return m_name; }
    QString path() { return m_path; }
    QString mainTable() { return m_mainTable; }
    QString tocTable() { return m_tocTable; }
    QString info() { return m_info; }

    void genInfo();
    void genInfo(ShamelaIndexInfo *info);

    TCHAR *idT() { return m_idT; }
    TCHAR *authorDeathT() { return m_authorDeathT; }

    void setId(int id);
    void setArchive(int archive);
    void setCat(int cat);
    void setAuthorID(int id);
    void setBookVersion(int ver);
    void setName(const QString &name);
    void setPath(const QString &path);
    void setAuthorDeath(int dYear);
    void setInfo(QString &info);

protected:
    void init();

protected:
    int m_id;
    QString m_name;
    QString m_path;
    QString m_mainTable;
    QString m_tocTable;
    QString m_info;
    int m_archive;
    int m_cat;
    int m_authorID;
    int m_authorDeath;
    int m_bookVersion;
    TCHAR *m_idT;
    TCHAR *m_authorDeathT;
};

#endif // BOOKINFO_H
