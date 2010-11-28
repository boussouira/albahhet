#ifndef INDEXINFO_H
#define INDEXINFO_H

#include <qstring.h>

class IndexInfo
{
public:
    IndexInfo();
    void setName(QString name);
    void setPath(QString path);
    void setShamelaPath(QString path);
    void setOptimizeIndex(bool optimize);
    void setRamSize(int size);

    QString name();
    QString path();
    QString shamelaPath();
    bool optimize();
    int ramSize();
    QString nameHash();
    static QString nameHash(QString name);

    QString shamelaDbName();
    QString shamelaDbPath();
    bool isShamelaPath(QString path);

    QString indexDbName();
    QString indexDbPath();

    QString buildFilePath(QString bkid, int archive);


protected:
    QString m_name;
    QString m_path;
    QString m_shamelaPath;
    bool m_optimizeIndex;
    int m_ramSize;
};

#endif // INDEXINFO_H
