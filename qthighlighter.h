#ifndef QTHIGHLIGHTER_H
#define QTHIGHLIGHTER_H

#include <qregexp.h>
#include <qstringlist.h>

class QtHighLighter
{
public:
    QtHighLighter();
    void clear();

    QString hiText(const QString &text, const QString &strToHi);
    QString hiText(const QString &text);
    QStringList buildRegExp(const QString &str);
    QString abbreviate(QString str, int size);
    QString cleanString(QString str);

    void setMaxTextLenght(int len);
    void setAbbrevite(bool abbrivate);
    void setStringToHighlight(QString str);

protected:
    QList<QString> m_colors;
    QString m_strToHighLight;
    QStringList m_regExpList;
    int m_maxTextLen;
    bool m_abbreviate;
};

#endif // QTHIGHLIGHTER_H
