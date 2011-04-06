#ifndef INDEXINFO_H
#define INDEXINFO_H

#include <qstring.h>

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

class IndexInfo
{
public:

    enum IndexType {
        ShamelaIndex = 1
    };

    IndexInfo();
    ~IndexInfo();
    void setID(int id);
    void setType(IndexType type);
    void setName(QString name);
    void setPath(QString path);
    void setShamelaPath(QString path);
    void setOptimizeIndex(bool optimize);
    void setRamSize(int size);
    void setIndexingInfo(IndexingInfo *info);

    int id();
    IndexType type();
    QString name();
    QString path();
    QString shamelaPath();
    bool optimize();
    int ramSize();
    IndexingInfo *indexingInfo();

    QString shamelaMainDbName();
    QString shamelaMainDbPath();
    QString shamelaSpecialDbName();
    QString shamelaSpecialDbPath();
    bool isShamelaPath(QString path);

    QString indexDbName();
    QString indexDbPath();

    QString buildFilePath(QString bkid, int archive);
    QString buildFilePath(int bkid, int archive);

    void generateIndexingInfo();

protected:
    int m_id;
    IndexType m_type;
    QString m_name;
    QString m_path;
    QString m_shamelaPath;
    bool m_optimizeIndex;
    int m_ramSize;
    IndexingInfo *m_indexingInfo;
};

#endif // INDEXINFO_H
