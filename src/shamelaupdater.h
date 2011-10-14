#ifndef SHAMELAUPDATER_H
#define SHAMELAUPDATER_H

#include <qstringlist.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qstandarditemmodel.h>

#include "indexinfo.h"
class ShamelaUpdaterTask
{
public:
    ShamelaUpdaterTask() {}

    enum Task {
        Add,
        Delete,
        Update
    };

    int bookID;
    int bookVersion;
    QString bookName;
    Task task;

    bool operator==(const ShamelaUpdaterTask &s) const;
    QString toString();
    void fromString(QString text);
};

class ShamelaUpdater
{
public:
    ShamelaUpdater();
    ~ShamelaUpdater();

    void setIndexInfo(IndexInfo *index);
    void close();

    void loadBooks();
    void addTask(ShamelaUpdaterTask task);
    bool removeTask(ShamelaUpdaterTask task);

    QList<QStandardItem*> getTaskItems();
    QList<int> getBooksToAdd();
    QList<int> getBooksToDelete();

    static const int taskStringRole = Qt::UserRole + 2;

protected:
    void openDB();

protected:
    QSqlDatabase m_indexDB;
    QSqlDatabase m_shamelaDB;
    QSqlQuery *m_query;
    QSqlQuery *m_shamelaQuery;
    IndexInfo *m_indexInfo;
    QString connName;
    QList<ShamelaUpdaterTask> m_tasks;
};

#endif // SHAMELAUPDATER_H
