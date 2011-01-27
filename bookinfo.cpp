#include "bookinfo.h"
#include <stdlib.h>

BookInfo::BookInfo()
{
    init();
}

BookInfo::BookInfo(int id, QString name, QString path, int archive):
        m_id(id),
        m_name(name),
        m_path(path),
        m_archive(archive)
{
    init();

    m_idT = _itow(id, m_idT, 10);
    m_archiveT = _itow(archive, m_archiveT, 10);
}

BookInfo::~BookInfo()
{
    delete [] m_idT;
    delete [] m_archiveT;
    delete [] m_authorIDT;
    delete [] m_catT;
}

void BookInfo::init()
{
    m_idT = new TCHAR[10];
    m_archiveT = new TCHAR[5];
    m_catT = new TCHAR[10];
    m_authorIDT = new TCHAR[10];
}

void BookInfo::setId(int id)
{
    m_id = id;
    m_idT = _itow(id, m_idT, 10);
}

void BookInfo::setArchive(int archive)
{
    m_archive = archive;
    m_archiveT = _itow(archive, m_archiveT, 10);
}

void BookInfo::setCat(int cat)
{
    m_cat = cat;
    m_catT = _itow(cat, m_catT, 10);
}

void BookInfo::setAuthorID(int id)
{
    m_authorID = id;
    m_authorIDT = _itow(id, m_authorIDT, 10);
}

void BookInfo::setName(const QString &name)
{
    m_name = name;
}

void BookInfo::setPath(const QString &path)
{
    m_path = path;
}
