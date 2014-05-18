#include "shamelaindexinfo.h"
#include "common.h"
#include <qdir.h>

ShamelaIndexInfo::ShamelaIndexInfo()
{
    m_indexingInfo = 0;
}

ShamelaIndexInfo::~ShamelaIndexInfo()
{
    if(m_indexingInfo)
        delete m_indexingInfo;
}

void ShamelaIndexInfo::setShamelaPath(QString path)
{
    QDir dir(path);
    m_shamelaPath = dir.absolutePath();
}

QString ShamelaIndexInfo::shamelaPath()
{
    return m_shamelaPath;
}

QString ShamelaIndexInfo::shamelaMainDbName()
{
    return QString("main.mdb");
}

QString ShamelaIndexInfo::shamelaMainDbPath()
{
    QDir dir(shamelaPath());
    dir.cd("Files");
    return dir.filePath(shamelaMainDbName());
}

QString ShamelaIndexInfo::shamelaSpecialDbName()
{
    return QString("special.mdb");
}

QString ShamelaIndexInfo::shamelaSpecialDbPath()
{
    QDir dir(shamelaPath());
    dir.cd("Files");
    return dir.filePath(shamelaSpecialDbName());
}

bool ShamelaIndexInfo::isShamelaPath(QString path)
{
    QDir dir(path);
    if(dir.cd("Files"))
        return dir.exists(shamelaMainDbName());

    return false;
}

QString ShamelaIndexInfo::shamelaBinPath()
{
    QDir dir(shamelaPath());
    dir.cd("bin");
    return dir.absolutePath();
}

QString ShamelaIndexInfo::shamelaAppPath()
{
    QDir dir(shamelaPath());
    dir.cd("bin");
    return dir.filePath("shamela.exe");
}

QString ShamelaIndexInfo::viewerAppPath()
{
    QDir dir(shamelaPath());
    dir.cd("bin");
    return dir.filePath("viewer.exe");
}

QString ShamelaIndexInfo::buildFilePath(QString bkid, int archive)
{
    if(!archive)
        return QString("%1/Books/%2/%3.mdb").arg(m_shamelaPath).arg(bkid.right(1)).arg(bkid);
    else
        return QString("%1/Books/Archive/%2.mdb").arg(m_shamelaPath).arg(archive);
}

QString ShamelaIndexInfo::buildFilePath(int bkid, int archive)
{
    return buildFilePath(QString::number(bkid), archive);
}


void ShamelaIndexInfo::setIndexingInfo(IndexingInfo *info)
{
    m_indexingInfo = info;
}

IndexingInfo *ShamelaIndexInfo::indexingInfo()
{
    return m_indexingInfo;
}

void ShamelaIndexInfo::generateIndexingInfo()
{
    if(!m_indexingInfo)
        m_indexingInfo = new IndexingInfo;
    m_indexingInfo->indexSize = getIndexSize(indexPath());
    m_indexingInfo->shamelaSize = getBooksSize(m_shamelaPath);
}

QDomElement ShamelaIndexInfo::toDomElement(QDomDocument &doc)
{
    QDomElement indexNode = doc.createElement("index");
    indexNode.setAttribute("id", id());
    indexNode.setAttribute("type", type());

    QDomElement nameNode = doc.createElement("name");
    nameNode.appendChild(doc.createTextNode(name()));
    indexNode.appendChild(nameNode);

    QDomElement shaPathNode = doc.createElement("shamel-path");
    shaPathNode.appendChild(doc.createTextNode(shamelaPath()));
    indexNode.appendChild(shaPathNode);

    QDomElement indexPathNode = doc.createElement("index-path");
    indexPathNode.appendChild(doc.createTextNode(path()));
    indexNode.appendChild(indexPathNode);


    // Indexing info
    if(m_indexingInfo) {
        QDomElement indexingInfoNode = doc.createElement("info");

        QDomElement createTime = doc.createElement("create-time");
        createTime.appendChild(doc.createTextNode(QString::number(m_indexingInfo->creatTime)));
        indexingInfoNode.appendChild(createTime);

        QDomElement indexingTime = doc.createElement("indexing-toke");
        indexingTime.appendChild(doc.createTextNode(QString::number(m_indexingInfo->indexingTime)));
        indexingInfoNode.appendChild(indexingTime);

        QDomElement optimizingTime = doc.createElement("optimizing-toke");
        optimizingTime.appendChild(doc.createTextNode(QString::number(m_indexingInfo->optimizingTime)));
        indexingInfoNode.appendChild(optimizingTime);

        QDomElement indexSize = doc.createElement("index-size");
        indexSize.appendChild(doc.createTextNode(QString::number(m_indexingInfo->indexSize)));
        indexingInfoNode.appendChild(indexSize);

        QDomElement shamelaSize = doc.createElement("shamela-size");
        shamelaSize.appendChild(doc.createTextNode(QString::number(m_indexingInfo->shamelaSize)));
        indexingInfoNode.appendChild(shamelaSize);

        indexNode.appendChild(indexingInfoNode);
    }

    return indexNode;
}


void ShamelaIndexInfo::fromDomElement(QDomElement &indexElement)
{
    setID(indexElement.attribute("id").toInt());
    setType((IndexInfoBase::IndexType) indexElement.attribute("type").toInt());
    setName(indexElement.firstChildElement("name").text());
    setShamelaPath(indexElement.firstChildElement("shamel-path").text());
    setPath(indexElement.firstChildElement("index-path").text());

    // Index info
    QDomNodeList infoList = indexElement.elementsByTagName("info");
    if(infoList.count() > 0) {
        QDomElement infoElement = infoList.at(0).toElement();
        IndexingInfo *indexingInfo = new IndexingInfo;
        indexingInfo->creatTime = infoElement.firstChildElement("create-time").text().toUInt();
        indexingInfo->indexingTime = infoElement.firstChildElement("indexing-toke").text().toInt();
        indexingInfo->optimizingTime = infoElement.firstChildElement("optimizing-toke").text().toInt();
        indexingInfo->indexSize = infoElement.firstChildElement("index-size").text().toULongLong();
        indexingInfo->shamelaSize = infoElement.firstChildElement("shamela-size").text().toULongLong();

        setIndexingInfo(indexingInfo);
    }
}
