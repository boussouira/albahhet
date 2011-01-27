#include "indexinfo.h"
#include <qdir.h>

IndexInfo::IndexInfo()
{
    m_optimizeIndex = true;
    m_ramSize = 0;
}

void IndexInfo::setName(QString name)
{
    m_name = name;
}

void IndexInfo::setPath(QString path)
{
    m_path = path;
    if(m_path.endsWith(QDir::separator()))
        m_path.resize(m_path.size()-1);
}

void IndexInfo::setShamelaPath(QString path)
{
    m_shamelaPath = path;
    if(m_shamelaPath.endsWith(QDir::separator()))
        m_shamelaPath.resize(m_shamelaPath.size()-1);
}

void IndexInfo::setOptimizeIndex(bool optimize)
{
    m_optimizeIndex = optimize;
}

void IndexInfo::setRamSize(int size)
{
    m_ramSize = size;
}

QString IndexInfo::name()
{
    return m_name;
}

QString IndexInfo::path()
{
    return m_path;
}

QString IndexInfo::shamelaPath()
{
    return m_shamelaPath;
}

bool IndexInfo::optimize()
{
    return m_optimizeIndex;
}

int IndexInfo::ramSize()
{
    return m_ramSize;
}

QString IndexInfo::nameHash()
{
    return nameHash(m_name);
}

QString IndexInfo::nameHash(QString name)
{
    return QString::number(qChecksum(qPrintable(name), name.size()+1));
}

QString IndexInfo::shamelaMainDbName()
{
    return QString("main.mdb");
}

QString IndexInfo::shamelaMainDbPath()
{
    return QString("%1/Files/%2").arg(shamelaPath()).arg(shamelaMainDbName());
}

QString IndexInfo::shamelaSpecialDbName()
{
    return QString("special.mdb");
}

QString IndexInfo::shamelaSpecialDbPath()
{
    return QString("%1/Files/%2").arg(shamelaPath()).arg(shamelaSpecialDbName());
}

bool IndexInfo::isShamelaPath(QString path)
{
    return QFile::exists(QString("%1/Files/%2").arg(path).arg(shamelaMainDbName()));
}

QString IndexInfo::indexDbName()
{
    return QString("book_index.db");
}

QString IndexInfo::indexDbPath()
{
    return QString("%1/%2").arg(path()).arg(indexDbName());

}

QString IndexInfo::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(shamelaPath()).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(shamelaPath()).arg(archive);
}
