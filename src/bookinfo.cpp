#include "bookinfo.h"
#include "shamelaindexinfo.h"
#include <stdlib.h>

BookInfo::BookInfo()
{
    init();
}

BookInfo::BookInfo(int bid, QString bname, QString bpath, int barchive):
        m_id(bid),
        m_name(bname),
        m_path(bpath),
        m_archive(barchive)
{
    init();

    m_idT = _itow(bid, m_idT, 10);
}

BookInfo::~BookInfo()
{
    free(m_idT);
    free(m_authorDeathT);
}

void BookInfo::init()
{
    m_idT           = (TCHAR*) malloc(sizeof(TCHAR) * 10);
    m_authorDeathT  = (TCHAR*) malloc(sizeof(TCHAR) * 10);
}

void BookInfo::genInfo()
{
    m_mainTable = (m_archive) ? QString("b%1").arg(m_id) : "book";
    m_tocTable = (m_archive) ? QString("t%1").arg(m_id) : "title";
}

void BookInfo::genInfo(ShamelaIndexInfo *info)
{
    genInfo();

    m_path = info->buildFilePath(QString::number(m_id), m_archive);
}

void BookInfo::setId(int id)
{
    m_id = id;
    m_idT = _itow(id, m_idT, 10);
}

void BookInfo::setArchive(int archive)
{
    m_archive = archive;
}

void BookInfo::setCat(int cat)
{
    m_cat = cat;
}

void BookInfo::setAuthorID(int id)
{
    m_authorID = id;
}

void BookInfo::setName(const QString &name)
{
    m_name = name;
}

void BookInfo::setPath(const QString &path)
{
    m_path = path;
}

void BookInfo::setAuthorDeath(int dYear)
{
    m_authorDeath = dYear;
    m_authorDeathT = _itow(dYear, m_authorDeathT, 10);
}

void BookInfo::setInfo(QString &info)
{
    m_info = info;
}

void BookInfo::setBookVersion(int ver)
{
    m_bookVersion = ver;
}
