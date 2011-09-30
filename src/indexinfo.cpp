#include "indexinfo.h"
#include "common.h"
#include <qdir.h>

IndexInfo::IndexInfo()
{
    m_id = -1;
    m_optimizeIndex = true;
    m_ramSize = 0;
    m_indexingInfo = 0;
    m_indexDirName = "index";
}

IndexInfo::~IndexInfo()
{
    if(m_indexingInfo)
        delete m_indexingInfo;
}

void IndexInfo::setName(QString name)
{
    m_name = name;
}

void IndexInfo::setPath(QString path)
{
    QDir dir(path);

    if(!dir.exists(m_indexDirName))
        dir.mkdir(m_indexDirName);

    m_path = dir.absolutePath();
}

void IndexInfo::setShamelaPath(QString path)
{
    QDir dir(path);
    m_shamelaPath = dir.absolutePath();
}


QString IndexInfo::name()
{
    return m_name;
}

QString IndexInfo::path()
{
    return m_path;
}

QString IndexInfo::indexPath()
{
    QDir dir(m_path);
    return dir.filePath(m_indexDirName);
}

QString IndexInfo::shamelaPath()
{
    return m_shamelaPath;
}

QString IndexInfo::shamelaMainDbName()
{
    return QString("main.mdb");
}

QString IndexInfo::shamelaMainDbPath()
{
    QDir dir(shamelaPath());
    dir.cd("Files");
    return dir.filePath(shamelaMainDbName());
}

QString IndexInfo::shamelaSpecialDbName()
{
    return QString("special.mdb");
}

QString IndexInfo::shamelaSpecialDbPath()
{
    QDir dir(shamelaPath());
    dir.cd("Files");
    return dir.filePath(shamelaSpecialDbName());
}

bool IndexInfo::isShamelaPath(QString path)
{
    QDir dir(path);
    if(dir.cd("Files"))
        return dir.exists(shamelaMainDbName());

    return false;
}

QString IndexInfo::indexDbName()
{
    return QString("book_index.db");
}

QString IndexInfo::indexDbPath()
{
    QDir dir(m_path);
    return dir.filePath(indexDbName());
}

QString IndexInfo::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(m_shamelaPath).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(m_shamelaPath).arg(archive);
}

QString IndexInfo::buildFilePath(int bkid, int archive)
{
    return buildFilePath(QString::number(bkid), archive);
}

void IndexInfo::setType(IndexInfo::IndexType type)
{
    m_type = type;
}

IndexInfo::IndexType IndexInfo::type()
{
    return m_type;
}

void IndexInfo::setID(int id)
{
    m_id = id;
}

int IndexInfo::id()
{
    return m_id;
}

void IndexInfo::setIndexingInfo(IndexingInfo *info)
{
    m_indexingInfo = info;
}

IndexingInfo *IndexInfo::indexingInfo()
{
    return m_indexingInfo;
}

void IndexInfo::generateIndexingInfo()
{
    if(!m_indexingInfo)
        m_indexingInfo = new IndexingInfo;
    m_indexingInfo->indexSize = getIndexSize(m_path);
    m_indexingInfo->shamelaSize = getBooksSize(m_shamelaPath);
}
