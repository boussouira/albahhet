#include "bookinfo.h"
#include "indexinfo.h"
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
    free(m_idT);
    free(m_archiveT);
    free(m_authorIDT);
    free(m_catT);
}

void BookInfo::init()
{
    m_idT       = (TCHAR*) malloc(sizeof(TCHAR) * 10);
    m_archiveT  = (TCHAR*) malloc(sizeof(TCHAR) * 10);
    m_catT      = (TCHAR*) malloc(sizeof(TCHAR) * 10);
    m_authorIDT = (TCHAR*) malloc(sizeof(TCHAR) * 10);
}

void BookInfo::genInfo()
{
    m_mainTable = (m_archive) ? QString("b%1").arg(m_id) : "book";
    m_tocTable = (m_archive) ? QString("t%1").arg(m_id) : "title";
}

void BookInfo::genInfo(IndexInfo *info)
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

void BookInfo::debug()
{
    qDebug() /*<< QDebug::nospace()*/
            << "(ID:" << id()
            << ", Archive:" << archive()
            << ", Path:" << path()
            << ", Tables('" << mainTable() << "', '" << tocTable() << "')"
            << "Name:" << name()
            << ")";
}
