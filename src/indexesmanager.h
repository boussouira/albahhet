#ifndef INDEXESMANAGER_H
#define INDEXESMANAGER_H

#include "common.h"
#include <QtXml>
#include <qsettings.h>

class IndexesManager
{
public:
    IndexesManager();
    ~IndexesManager();
    int count();
    void add(IndexInfo *index);
    void add(QSettings &settings, QString name);
    void remove(IndexInfo *index);
    void update(IndexInfo *index);
    void setIndexName(IndexInfo *index, QString name);
    IndexInfo *indexInfo(int indexID);
    QList<IndexInfo*> list();
    bool nameExists(QString name);
    bool idExists(int id);
    void clear();

protected:
    void generateIndexesList();
    void createFile();
    void checkFile();
    void save();

protected:
    QDomDocument m_doc;
    QDomElement m_rootElement;
    QString m_path;
    QList<IndexInfo*> m_list;
    QHash<int, IndexInfo*> m_indexInfoMap;
};

#endif // INDEXESMANAGER_H
