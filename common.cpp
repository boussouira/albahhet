#include "common.h"
#include <qstringlist.h>

TCHAR* QStringToTChar(const QString &str)
{
    //    TCHAR *string = new TCHAR[(str.length() +1) * sizeof(TCHAR)];
    //    memset(string, 0, (str.length() +1) * sizeof(TCHAR));
    TCHAR *string = new TCHAR[str.length()+1];
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
        list <<  QObject::trUtf8("ثانية")
        << QObject::trUtf8("ثانيتين")
        << QObject::trUtf8("ثوان")
        << QObject::trUtf8("ثانية");
    else if(word == MINUTE)
        list <<  QObject::trUtf8("دقيقة")
        << QObject::trUtf8("دقيقتين")
        << QObject::trUtf8("دقائق")
        << QObject::trUtf8("دقيقة");
    else if(word == HOUR)
        list <<  QObject::trUtf8("ساعة")
        << QObject::trUtf8("ساعتين")
        << QObject::trUtf8("ساعات")
        << QObject::trUtf8("ساعة");
    else if(word == BOOK)
        list <<  QObject::trUtf8("كتاب واحد")
        << QObject::trUtf8("كتابين")
        << QObject::trUtf8("كتب")
        << QObject::trUtf8("كتابا");

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
    text.replace(QRegExp(QObject::trUtf8("ـفق")), "(");
    text.replace(QRegExp(QObject::trUtf8("ـغق")), ")");
    text.replace(QRegExp(QObject::trUtf8("ـ[أا]و")), "OR");
    text.replace(QRegExp(QObject::trUtf8("ـو")), "AND");
    text.replace(QRegExp(QObject::trUtf8("ـبدون")), "NOT");
    text.replace(QObject::trUtf8("؟"), "?");
}

void hideHelpButton(QWidget *w)
{
    Qt::WindowFlags flags = w->windowFlags();
    flags |= Qt::WindowContextHelpButtonHint;
    flags ^= Qt::WindowContextHelpButtonHint;

    w->setWindowFlags(flags);
}

void forceRTL(QWidget *widget)
{
    widget->setLayoutDirection(Qt::LeftToRight);
    widget->setLayoutDirection(Qt::RightToLeft);
}

void clearShorts(QString &str)
{
    str.replace('A', QObject::trUtf8("صلى الله عليه وسلم"));
    str.replace('B', QObject::trUtf8("رضي الله عن"));
    str.replace('C', QObject::trUtf8("رحمه الله"));
    str.replace('D', QObject::trUtf8("عز وجل"));
    str.replace('E', QObject::trUtf8("عليه الصلاة و السلام"));
}
