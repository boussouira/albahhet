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
        ShamelaIndex = 1,
        QuranIndex
    };

    IndexInfo();
    ~IndexInfo();
    void setID(int id);
    void setType(IndexType type);
    void setName(QString name);
    void setPath(QString path);
    void setShamelaPath(QString path);
    void setIndexingInfo(IndexingInfo *info);

    int id();
    IndexType type();
    QString name();
    QString path();
    QString indexPath();
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
    QString m_indexDirName;
};

#endif // INDEXINFO_H
