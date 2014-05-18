#include "indexinfobase.h"
#include "common.h"
#include <QDir>

IndexInfoBase::IndexInfoBase()
{
    m_id = -1;
    m_indexDirName = "index";
}

void IndexInfoBase::setID(int id)
{
    m_id = id;
}

void IndexInfoBase::setType(IndexInfoBase::IndexType type)
{
    m_type = type;
}


void IndexInfoBase::setName(QString name)
{
    m_name = name;
}

void IndexInfoBase::setPath(QString path)
{
    QDir dir(path);

    if(!dir.exists(m_indexDirName))
        dir.mkdir(m_indexDirName);

    m_path = dir.absolutePath();
}

int IndexInfoBase::id()
{
    return m_id;
}

IndexInfoBase::IndexType IndexInfoBase::type()
{
    return m_type;
}

QString IndexInfoBase::name()
{
    return m_name;
}

QString IndexInfoBase::path()
{
    return m_path;
}

QString IndexInfoBase::indexPath()
{
    QDir dir(m_path);
    return dir.filePath(m_indexDirName);
}

QString IndexInfoBase::indexDbName()
{
    return QString("book_index.db");
}

QString IndexInfoBase::indexDbPath()
{
    QDir dir(m_path);
    return dir.filePath(indexDbName());
}

QDomElement IndexInfoBase::toDomElement(QDomDocument &doc)
{
    Q_UNUSED(doc);

    return QDomElement();
}

void IndexInfoBase::fromDomElement(QDomElement &indexElement)
{
    Q_UNUSED(indexElement);
}
