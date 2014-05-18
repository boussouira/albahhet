#include "common.h"
#include <qstringlist.h>
#include <QTime>
#include <qsettings.h>
#include <quuid.h>

void numberSrand()
{
    srand(QDateTime::currentDateTime().toTime_t());
}

int numberRand(int smin, int smax)
{
    int rVal = (smin + (rand() % (smax-smin+1)));
    return qBound(smin, rVal, smax);
}

TCHAR* QStringToTChar(const QString &str)
{
    TCHAR *string = (TCHAR*) malloc((str.length()+1) * sizeof(TCHAR));
    str.toWCharArray(string);
    string[str.length()] = 0;

    return string;
}

QString TCharToQString(const TCHAR *string)
{
    return QString::fromWCharArray(string);
}

QString arPlural(int count, PULRAL word, bool html)
{
    QStringList list;
    QString str;

    if(word == SECOND)
        list <<  QObject::tr("ثانية")
        << QObject::tr("ثانيتين")
        << QObject::tr("ثوان")
        << QObject::tr("ثانية");
    else if(word == MINUTE)
        list <<  QObject::tr("دقيقة")
        << QObject::tr("دقيقتين")
        << QObject::tr("دقائق")
        << QObject::tr("دقيقة");
    else if(word == HOUR)
        list <<  QObject::tr("ساعة")
        << QObject::tr("ساعتين")
        << QObject::tr("ساعات")
        << QObject::tr("ساعة");
    else if(word == BOOK)
        list <<  QObject::tr("كتاب واحد")
        << QObject::tr("كتابين")
        << QObject::tr("كتب")
        << QObject::tr("كتابا");

    if(count <= 1)
        str = list.at(0);
    else if(count == 2)
        str = list.at(1);
    else if (count > 2 && count <= 10)
        str = QString("%1 %2").arg(count).arg(list.at(2));
    else if (count > 10)
        str = QString("%1 %2").arg(count).arg(list.at(3));
    else
        str = QString();

    return html ? QString("<strong>%1</strong>").arg(str) : str;
}

void normaliseSearchString(QString &text)
{
    text.replace(QRegExp(QObject::tr("\\b" "ـ[أا]و" "\\b")), "OR");
    text.replace(QRegExp(QObject::tr("\\b" "ـو" "\\b")), "AND");
    text.replace(QRegExp(QObject::tr("\\b" "ـبدون" "\\b")), "NOT");
    text.replace(QObject::tr("؟"), "?");
}

void hideWindowButtons(QWidget *w, bool helpButton, bool closeButton)
{
    Qt::WindowFlags flags = w->windowFlags();

    if(helpButton) {
        flags |= Qt::WindowContextHelpButtonHint;
        flags ^= Qt::WindowContextHelpButtonHint;
    }

    if(closeButton) {
        flags |= Qt::CustomizeWindowHint;

        flags |= Qt::WindowCloseButtonHint;
        flags ^= Qt::WindowCloseButtonHint;
    }

    w->setWindowFlags(flags);
}

void forceRTL(QWidget *widget)
{
    widget->setLayoutDirection(Qt::LeftToRight);
    widget->setLayoutDirection(Qt::RightToLeft);
}

void clearShorts(QString &str)
{
    str.replace(QRegExp(QObject::tr("([\\x0621-\\x06ED]\\W)A\\b")), QObject::tr("\\1""صلى الله عليه وسلم"));
    str.replace(QRegExp(QObject::tr("([\\x0621-\\x06ED]\\W)B\\b")), QObject::tr("\\1""رضي الله عن"));
    str.replace(QRegExp(QObject::tr("([\\x0621-\\x06ED]\\W)C\\b")), QObject::tr("\\1""رحمه الله"));
    str.replace(QRegExp(QObject::tr("([\\x0621-\\x06ED]\\W)D\\b")), QObject::tr("\\1""عز وجل"));
    str.replace(QRegExp(QObject::tr("([\\x0621-\\x06ED]\\W)E\\b")), QObject::tr("\\1""عليه الصلاة و السلام"));
}

quint64 getIndexSize(const QString &path)
{
    QDir dir;
    dir.cd(path);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);

    QFileInfoList list = dir.entryInfoList();
    quint64 size = 0;

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        size += fileInfo.size();
    }

    return size;
}

quint64 getBooksSize(const QString &shamelaPath)
{
    return getDirSize(shamelaPath);
}

quint64 getDirSize(const QString &path)
{
    QFileInfo info(path);
    quint64 size = 0;

    if(info.isDir()){
        QDir dir(path);
        foreach(QFileInfo fieInfo, dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot)) {
            if(fieInfo.isFile() && fieInfo.suffix() == "mdb")
                size += fieInfo.size();
            else if(fieInfo.isDir())
                size += getDirSize(fieInfo.absoluteFilePath());
        }
    }

    return size;
}

QString getSizeString(quint64 size)
{
    QString sizeStr;

    if(size < 1024)
        sizeStr = QObject::tr("%1 بيت").arg(size);
    else if(1024 <= size && size < 1024*1024)
        sizeStr = QObject::tr("%1 كيلو").arg(size/(1024.0), 4);
    else if( 1024*1024 <= size && size < 1024*1024*1024)
        sizeStr = QObject::tr("%1 ميغا").arg(size/(1024.0*1024.0), 4);
    else
        sizeStr = QObject::tr("%1 جيجا").arg(size/(1024.0*1024.0*1024.0), 4);

    return sizeStr;
}

QString getTimeString(int milsec, bool html)
{
    QString time;

    int seconde = (int) ((milsec / 1000) % 60);
    int minutes = (int) (((milsec / 1000) / 60) % 60);
    int hours   = (int) (((milsec / 1000) / 60) / 60);

    if(hours > 0){
        time.append(arPlural(hours, HOUR, html));
        time.append(QObject::tr(" و "));
    }

    if(minutes > 0 || hours > 0) {
        time.append(arPlural(minutes, MINUTE, html));
        time.append(QObject::tr(" و "));
    }

    time.append(arPlural(seconde, SECOND, html));

    return time;
}

QString hijriYear(int hYear)
{
    if(hYear <= 0)
        return QObject::tr("%1 م").arg(hijriToGregorian(hYear));
    else if(hYear >= 99999)
        return QObject::tr("معاصر");
    else
        return QObject::tr("%1 هـ").arg(hYear);
}

int hijriToGregorian(int hYear)
{
    return (hYear + 622) - (hYear / 33);
}

int gregorianToHijri(int gYear)
{
    return  (gYear - 622) + ((gYear - 622) / 32);
}

QString userId()
{
    QSettings settings;
    settings.beginGroup("info");

    if(!settings.contains("uid")) {
        settings.setValue("uid", QUuid::createUuid().toString());
    }

    return settings.value("uid").toString();
}
