#ifndef SEARCHFIELD_H
#define SEARCHFIELD_H

#include <qstringlist.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>

#include "indexinfobase.h"

class SearchFieldInfo {
public:
    SearchFieldInfo() {}
    QString name;
    int fieldID;
};

class SearchField
{
public:
    SearchField();
    ~SearchField();

    void setIndexInfo(IndexInfoBase *index);
    void close();
    QList<int> getFieldBooks(int fid);
    void removeield(int fid);
    void setFieldName(QString name, int fid);
    bool addField(int indexID, QString name, QList<int> list);
    QList<SearchFieldInfo*> getFieldsNames(int fid);
    QList<SearchFieldInfo*> getFieldsNames();

protected:
    void openDB();

protected:
    QSqlDatabase m_indexDB;
    QSqlQuery *m_query;
    IndexInfoBase *m_indexInfo;
    QString connName;
};

#endif // SEARCHFIELD_H
