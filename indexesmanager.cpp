#include "indexesmanager.h"
#include <qfile.h>
#include <qdir.h>
#include <qdebug.h>
#include <qdesktopservices.h>

IndexesManager::IndexesManager()
{
    m_path = QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation)).filePath("indexes.xml");

    QString errorStr;
    int errorLine;
    int errorColumn;

    if(!QFile::exists(m_path))
        createFile();
    else
        checkFile();

    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Can not open file %s", qPrintable(m_path));
        return;
    }

    if (!m_doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
        qFatal("Parse error at line %d, column %d: %s", errorLine, errorColumn, qPrintable(errorStr));
        return;
    }


    m_rootElement = m_doc.documentElement();
    if (m_rootElement.tagName() != "indexes-list") {
        qFatal("No indexes list at %s", qPrintable(m_path));
        return;
    }

    generateIndexesList();

    /*
    QList<IndexInfo *> listx = list();
    foreach(IndexInfo *index, listx) {
        qDebug() << "*\tID:" << index->id() << "\n\t"
                 << "Type:" << index->type() << "\n\t"
                 << "Name:" << index->name() << "\n\t"
                 << "Shamela:" << index->shamelaPath() << "\n\t"
                 << "Index:" << index->path();
        IndexingInfo *info = index->indexingInfo();
        if(info) {
            qDebug() << "\t\tcreatTime:" << info->creatTime << "\n\t\t"
                     << "lastUpdate:" << info->lastUpdate << "\n\t\t"
                     << "indexingTime:" << info->indexingTime << "\n\t\t"
                     << "optimizingTime:" << info->optimizingTime << "\n\t\t"
                     << "indexSize:" << info->indexSize << "\n\t\t"
                     << "shamelaSize:" << info->shamelaSize;
        }
    }
    IndexInfo *info = listx.first();
    info->setName("XXx");
    info->setPath("yYy");
    info->setShamelaPath("zZz");
    setIndexName(info, "{[0]}");
    */
}

IndexesManager::~IndexesManager()
{
    clear();
}

int IndexesManager::count()
{
    return  m_rootElement.elementsByTagName("index").count();
}

void IndexesManager::add(IndexInfo *index)
{
    qDebug() << "Adding" << index->name() << "to xml file...";
    QDomDocumentFragment newIndex(m_doc.createDocumentFragment());

    int indexID =  m_rootElement.attribute("last-id", 0).toInt()+1;
    QDomElement indexNode(m_doc.createElement("index"));
    indexNode.setAttribute("id", indexID);
    indexNode.setAttribute("type", index->type());

    QDomElement nameNode(m_doc.createElement("name"));
    nameNode.appendChild(m_doc.createTextNode(index->name()));
    indexNode.appendChild(nameNode);

    QDomElement shaPathNode(m_doc.createElement("shamel-path"));
    shaPathNode.appendChild(m_doc.createTextNode(index->shamelaPath()));
    indexNode.appendChild(shaPathNode);

    QDomElement indexPathNode(m_doc.createElement("index-path"));
    indexPathNode.appendChild(m_doc.createTextNode(index->path()));
    indexNode.appendChild(indexPathNode);

    newIndex.appendChild(indexNode);

    // Indexing info
    if(index->indexingInfo()) {
        IndexingInfo *info = index->indexingInfo();
        QDomElement indexingInfoNode(m_doc.createElement("info"));

        QDomElement createTime(m_doc.createElement("create-time"));
        createTime.appendChild(m_doc.createTextNode(QString::number(info->creatTime)));
        indexingInfoNode.appendChild(createTime);

        QDomElement indexingTime(m_doc.createElement("indexing-toke"));
        indexingTime.appendChild(m_doc.createTextNode(QString::number(info->indexingTime)));
        indexingInfoNode.appendChild(indexingTime);

        QDomElement optimizingTime(m_doc.createElement("optimizing-toke"));
        optimizingTime.appendChild(m_doc.createTextNode(QString::number(info->optimizingTime)));
        indexingInfoNode.appendChild(optimizingTime);

        QDomElement indexSize(m_doc.createElement("index-size"));
        indexSize.appendChild(m_doc.createTextNode(QString::number(info->indexSize)));
        indexingInfoNode.appendChild(indexSize);

        QDomElement shamelaSize(m_doc.createElement("shamela-size"));
        shamelaSize.appendChild(m_doc.createTextNode(QString::number(info->shamelaSize)));
        indexingInfoNode.appendChild(shamelaSize);

        indexNode.appendChild(indexingInfoNode);
    }

    m_rootElement.appendChild(newIndex);
    m_rootElement.setAttribute("last-id", indexID);
    m_rootElement.setAttribute("count", m_rootElement.elementsByTagName("index").count());

    save();
}

void IndexesManager::add(QSettings &settings, QString name)
{

    IndexInfo info;

    settings.beginGroup(name);
    info.setType(IndexInfo::ShamelaIndex);
    info.setName(settings.value("name").toString());
    info.setShamelaPath(settings.value("shamela_path").toString());
    info.setPath(settings.value("index_path").toString());
    settings.endGroup();

    add(&info);
}

void IndexesManager::remove(IndexInfo *index)
{
    QDomNodeList nodesList = m_rootElement.elementsByTagName("index");

    for (int i=0; i<nodesList.count(); i++) {
        QDomElement indexElement = nodesList.at(i).toElement();

        if(indexElement.attribute("id").toInt() == index->id()) {
            qDebug("Delete index: %d", index->id());
            m_rootElement.removeChild(indexElement);
             m_rootElement.setAttribute("count", m_rootElement.elementsByTagName("index").count());
            save();

            break;
        }
    }
}

void IndexesManager::update(IndexInfo *index)
{
    QDomNodeList nodesList = m_rootElement.elementsByTagName("index");

    for (int i=0; i<nodesList.count(); i++) {
        QDomElement indexElement = nodesList.at(i).toElement();

        if(indexElement.attribute("id").toInt() == index->id()) {
            qDebug("Update index: %d", index->id());

            QDomDocumentFragment newIndex(m_doc.createDocumentFragment());

            QDomElement indexNode(m_doc.createElement("index"));;
            indexNode.setAttribute("id", indexElement.attribute("id").toInt());
            indexNode.setAttribute("type", indexElement.attribute("type").toInt());

            QDomElement nameNode(m_doc.createElement("name"));
            nameNode.appendChild(m_doc.createTextNode(index->name()));
            indexNode.appendChild(nameNode);

            QDomElement shaPathNode(m_doc.createElement("shamel-path"));
            shaPathNode.appendChild(m_doc.createTextNode(index->shamelaPath()));
            indexNode.appendChild(shaPathNode);

            QDomElement indexPathNode(m_doc.createElement("index-path"));
            indexPathNode.appendChild(m_doc.createTextNode(index->path()));
            indexNode.appendChild(indexPathNode);

            newIndex.appendChild(indexNode);

            m_rootElement.replaceChild(newIndex, indexElement);
            save();

            break;
        }
    }
}

void IndexesManager::setIndexName(IndexInfo *index, QString name)
{
    index->setName(name);
    update(index);
}

void IndexesManager::createFile()
{
    qDebug("Creating xml file...");
    QFile file(m_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setCodec("utf-8");

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n"
        << "<indexes-list count=\"0\" last-id=\"0\"></indexes-list>";
}

void IndexesManager::save()
{
    QFile file(m_path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("File not opened");
        return;
    }
    QTextStream out(&file);
    out.setCodec("utf-8");

    out << m_doc.toString(4);
}

void IndexesManager::checkFile()
{
    QFileInfo file(m_path);
    if(file.size() < 50)
        createFile();
}

QList<IndexInfo *> IndexesManager::list()
{
    if(m_list.isEmpty())
        generateIndexesList();

    return m_list;
}

void IndexesManager::generateIndexesList()
{
    QDomNodeList nodesList = m_rootElement.elementsByTagName("index");

    for (int i=0; i<nodesList.count(); i++) {
        IndexInfo *index = new IndexInfo();
        QDomElement indexElement = nodesList.at(i).toElement();
        index->setID(indexElement.attribute("id").toInt());
        index->setType((IndexInfo::IndexType) indexElement.attribute("type").toInt());
        index->setName(indexElement.firstChildElement("name").text());
        index->setShamelaPath(indexElement.firstChildElement("shamel-path").text());
        index->setPath(indexElement.firstChildElement("index-path").text());

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

            index->setIndexingInfo(indexingInfo);
        }

        m_list.append(index);
        m_indexInfoMap.insert(index->id(), index);
    }
}

void IndexesManager::clear()
{
    qDeleteAll(m_list);
    m_list.clear();
    m_indexInfoMap.clear();
}

IndexInfo *IndexesManager::indexInfo(int indexID)
{
    if(m_list.isEmpty())
        generateIndexesList();

    return m_indexInfoMap.value(indexID, 0);
}

bool IndexesManager::nameExists(QString name)
{
    if(m_list.isEmpty())
        generateIndexesList();

    foreach (IndexInfo *info, m_list) {
        if(info->name() == name)
            return true;
    }

    return false;
}

bool IndexesManager::idExists(int id)
{
    if(m_list.isEmpty())
        generateIndexesList();

    foreach (IndexInfo *info, m_list) {
        if(info->id() == id) {
            return true;
        }
    }

    qDebug("Index id: %d doesen't exists", id);
    return false;
}

