#ifndef SHAMELAINDEXINFO_H
#define SHAMELAINDEXINFO_H

#include "indexinfobase.h"

class IndexingInfo
{
public:
    IndexingInfo() {}

public:
    uint creatTime;
    uint lastUpdate;
    int indexingTime;
    int optimizingTime;
    quint64 indexSize;
    quint64 shamelaSize;
};

class ShamelaIndexInfo : public IndexInfoBase
{
public:
    ShamelaIndexInfo();
    ~ShamelaIndexInfo();
    void setShamelaPath(QString path);
    void setIndexingInfo(IndexingInfo *info);

    QString shamelaPath();
    IndexingInfo *indexingInfo();

    QString shamelaMainDbName();
    QString shamelaMainDbPath();
    QString shamelaSpecialDbName();
    QString shamelaSpecialDbPath();
    bool isShamelaPath(QString path);

    QString shamelaBinPath();
    QString shamelaAppPath();
    QString viewerAppPath();

    QString buildFilePath(QString bkid, int archive);
    QString buildFilePath(int bkid, int archive);

    void generateIndexingInfo();

    QDomElement toDomElement(QDomDocument &doc);
    void fromDomElement(QDomElement &indexElement);

protected:
    QString m_shamelaPath;
    IndexingInfo *m_indexingInfo;
};

#endif // SHAMELAINDEXINFO_H
