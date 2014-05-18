#ifndef INDEXINFOBASE_H
#define INDEXINFOBASE_H

#include <qstring.h>
#include <qdom.h>

class IndexInfoBase
{
public:

    enum IndexType {
        ShamelaIndex = 1,
        QuranIndex
    };

    IndexInfoBase();

    void setID(int id);
    void setType(IndexType type);
    void setName(QString name);
    void setPath(QString path);

    int id();
    IndexType type();
    QString name();
    QString path();
    QString indexPath();

    virtual QString indexDbName();
    QString indexDbPath();

    virtual QDomElement toDomElement(QDomDocument &doc);
    virtual void fromDomElement(QDomElement &indexElement);

protected:
    int m_id;
    IndexType m_type;
    QString m_name;
    QString m_path;
    QString m_indexDirName;
};

#endif // INDEXINFOBASE_H
