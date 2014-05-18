#include "indexesmanager.h"
#include "shamelaindexinfo.h"
#include <qfile.h>
#include <qdir.h>
#include <qdebug.h>
#include <qdesktopservices.h>

QDomNode findChildText(QDomElement &parent, QDomDocument &doc, bool cdata)
{
    QDomNode child = parent.firstChild();
    if(child.isNull()) {
        if(cdata)
            child = parent.appendChild(doc.createCDATASection(""));
        else
            child = parent.appendChild(doc.createTextNode(""));
    }

    return child;
}

void setElementText(QDomElement &parent, QDomDocument &doc, const QString &text, bool cdata)
{
    if(!parent.isNull()) {
        QDomNode textNode = findChildText(parent, doc, cdata);
        if(!textNode.isNull())
            textNode.setNodeValue(text);
        else
            qCritical("findChildText: Text node is null");
    } else {
        qCritical("findChildText: element is null");
    }
}

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

void IndexesManager::add(IndexInfoBase *index)
{
    qDebug() << "Adding" << index->name() << "to xml file...";

    int indexID = numberRand(1111, 9999);
    while (indexInfo(indexID)) {
        indexID = numberRand(1111, 9999);
    }

    QDomElement indexNode = index->toDomElement(m_doc);

    m_rootElement.appendChild(indexNode);
    m_rootElement.setAttribute("count", m_rootElement.elementsByTagName("index").count());

    save();
}

void IndexesManager::remove(IndexInfoBase *index)
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

void IndexesManager::update(IndexInfoBase *index)
{
    QDomNodeList nodesList = m_rootElement.elementsByTagName("index");

    for (int i=0; i<nodesList.count(); i++) {
        QDomElement indexElement = nodesList.at(i).toElement();

        if(indexElement.attribute("id").toInt() == index->id()) {
            qDebug("Update index: %d", index->id());

            setElementText(indexElement.firstChildElement("name"), m_doc, index->name(), false);
            save();

            break;
        }
    }
}

void IndexesManager::setIndexName(IndexInfoBase *index, QString name)
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
        << "<indexes-list count=\"0\"></indexes-list>";
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

QList<IndexInfoBase *> IndexesManager::list()
{
    if(m_list.isEmpty())
        generateIndexesList();

    return m_list;
}

void IndexesManager::generateIndexesList()
{
    QDomNodeList nodesList = m_rootElement.elementsByTagName("index");

    for (int i=0; i<nodesList.count(); i++) {
        IndexInfoBase *index = new ShamelaIndexInfo();
        index->fromDomElement(nodesList.at(i).toElement());

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

IndexInfoBase *IndexesManager::indexInfo(int indexID)
{
    if(m_list.isEmpty())
        generateIndexesList();

    return m_indexInfoMap.value(indexID, 0);
}

bool IndexesManager::nameExists(QString name)
{
    if(m_list.isEmpty())
        generateIndexesList();

    foreach (IndexInfoBase *info, m_list) {
        if(info->name() == name)
            return true;
    }

    return false;
}

bool IndexesManager::idExists(int id)
{
    if(m_list.isEmpty())
        generateIndexesList();

    foreach (IndexInfoBase *info, m_list) {
        if(info->id() == id) {
            return true;
        }
    }

    qDebug("Index id: %d doesen't exists", id);
    return false;
}

