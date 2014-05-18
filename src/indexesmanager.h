#ifndef INDEXESMANAGER_H
#define INDEXESMANAGER_H

#include "common.h"
#include "indexinfobase.h"
#include <QtXml>
#include <qsettings.h>

class IndexesManager
{
public:
    IndexesManager();
    ~IndexesManager();
    int count();
    void add(IndexInfoBase *index);
    void remove(IndexInfoBase *index);
    void setIndexName(IndexInfoBase *index, QString name);
    IndexInfoBase *indexInfo(int indexID);
    QList<IndexInfoBase*> list();
    bool nameExists(QString name);
    bool idExists(int id);
    void clear();

protected:
    void generateIndexesList();
    void createFile();
    void checkFile();
    void update(IndexInfoBase *index);
    void save();

protected:
    QDomDocument m_doc;
    QDomElement m_rootElement;
    QString m_path;
    QList<IndexInfoBase*> m_list;
    QHash<int, IndexInfoBase*> m_indexInfoMap;
};

#endif // INDEXESMANAGER_H
