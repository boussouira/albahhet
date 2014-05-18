#ifndef COMMON_H
#define COMMON_H

#include <qwidget.h>
#include <qdesktopservices.h>
#include <qdir.h>
#include <CLucene/SharedHeader.h>

#define LOG_FILE QDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation)).filePath("session.txt")

enum PULRAL{
    SECOND  = 1,
    MINUTE  = 2,
    HOUR    = 3,
    BOOK    = 4
};

void numberSrand();
int numberRand(int smin, int smax);

TCHAR* QStringToTChar(const QString &str);
QString TCharToQString(const TCHAR *string);

QString arPlural(int count, PULRAL word, bool html=false);

void normaliseSearchString(QString &text);

void hideWindowButtons(QWidget *w, bool helpButton=true, bool closeButton=false);
void clearShorts(QString &str);
void forceRTL(QWidget *widget);

quint64 getIndexSize(const QString &path);
quint64 getBooksSize(const QString &shamelaPath);
quint64 getDirSize(const QString &path);
QString getSizeString(quint64 size);
QString getTimeString(int milsec, bool html=true);

QString hijriYear(int hYear);

int hijriToGregorian(int hYear);
int gregorianToHijri(int gYear);

QString userId();

#define PROGRESS_DIALOG_STEP(text)     progress.setValue(progress.value()+1); \
                                progress.setLabelText(tr("جاري " text "..."));

#define DB_OPEN_ERROR(path) qCritical("[%s:%d] Cannot open database at \"%s\".", \
                                __FILE__, \
                                __LINE__, \
                                qPrintable(path));

#define SQL_ERROR(error) qCritical("[%s:%d] SQL error: \"%s\".", \
                                          __FILE__, \
                                          __LINE__, \
                                          qPrintable(error));

#define DELETE_DB(p) { QStringList connList = p->connections(); \
                        delete p; \
                        foreach(QString conn, connList){QSqlDatabase::removeDatabase(conn);}}


#endif // COMMON_H
