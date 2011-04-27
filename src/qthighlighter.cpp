#include "qthighlighter.h"
#include <qdebug.h>

QtHighLighter::QtHighLighter()
{
    m_colors.append("#ffff63");
    m_colors.append("#a5ffff");
    m_colors.append("#ff9a9c");
    m_colors.append("#9cff9c");
    m_colors.append("#ef86fb");

    m_abbreviate = false;
    m_maxTextLen = 320;
}

void QtHighLighter::clear()
{
    m_strToHighLight.clear();
    m_regExpList.clear();
}

QString QtHighLighter::cleanString(QString str)
{
    str.replace(QRegExp("[\\x0627\\x0622\\x0623\\x0625]"), "[\\x0627\\x0622\\x0623\\x0625]");//ALEFs
    str.replace(QRegExp("[\\x0647\\x0629]"), "[\\x0647\\x0629]"); //TAH_MARBUTA -> HEH

    return str;
}

QString QtHighLighter::hiText(const QString &text, const QString &strToHi)
{
    m_regExpList = buildRegExp(strToHi);
    QString finlStr  = text;

    if(m_abbreviate)
        finlStr = abbreviate(finlStr, m_maxTextLen);

    int color = 0;
    bool useColors = (m_regExpList.size() <= m_colors.size());

    foreach(QString regExp, m_regExpList)
        finlStr.replace(QRegExp(cleanString(regExp)),
                        QString("<b style=\"background-color:%1\">\\1</b>")
                        .arg(m_colors.at(useColors ? color++ : color)));

//    if(!useColors)
//        finlStr.replace(QRegExp("<\\/b>([\\s])<b style=\"background-color:[^\"]+\">"), "\\1");

    return finlStr;
}

QString QtHighLighter::hiText(const QString &text)
{
    QString finlStr = text;
    int color = 0;

    bool useColors = (m_regExpList.size() <= m_colors.size());

    if(m_abbreviate)
        finlStr = abbreviate(finlStr, m_maxTextLen);

    foreach(QString regExp, m_regExpList)
        finlStr.replace(QRegExp(cleanString(regExp)),
                        QString("<b style=\"background-color:%1\">\\1</b>")
                        .arg(m_colors.at(useColors ? color++ : color)));

    return finlStr;
}

QStringList QtHighLighter::buildRegExp(const QString &str)
{
    QString text = str;
    text.remove(QRegExp(QObject::tr("[\\x064B-\\x0652\\x0600\\x061B-\\x0620،]")));

    QStringList strWords = text.split(QRegExp(QObject::tr("[\\s;,.()\"'{}\\[\\]]")), QString::SkipEmptyParts);
    QStringList regExpList;
    QChar opPar('(');
    QChar clPar(')');
    foreach(QString word, strWords)
    {
        QString regExpStr;
        regExpStr.append("\\b");
        regExpStr.append(opPar);

        for (int i=0; i< word.size();i++) {
            if(word.at(i) == QChar('~'))
                regExpStr.append("[\\S]*");
            else if(word.at(i) == QChar('*'))
                regExpStr.append("[\\S]*");
            else if(word.at(i) == QChar('?'))
                regExpStr.append("\\S");
            else if( word.at(i) == QChar('"') || word.at(i) == opPar || word.at(i) == opPar )
                continue;
            else {
                regExpStr.append(word.at(i));
                regExpStr.append(QObject::tr("[\\x064B\\x064C\\x064D\\x064E\\x064F\\x0650\\x0651\\x0652\\x0653]*"));
            }
        }

        regExpStr.append(clPar);
        regExpStr.append("\\b");
        regExpList.append(regExpStr);
    }

    return regExpList;
}

QString QtHighLighter::abbreviate(QString str, int size) {
        if (str.length() <= size-3)
                return str;

        str.simplified();
        int index = str.lastIndexOf(' ', size-3);
        if (index <= -1)
                return "";
        return str.left(index).append("...");
}

void QtHighLighter::setMaxTextLenght(int len)
{
    m_maxTextLen = len;
}

void QtHighLighter::setAbbrevite(bool abbrivate)
{
    m_abbreviate = abbrivate;
}

void QtHighLighter::setStringToHighlight(QString str)
{
    m_strToHighLight = str;
    m_regExpList = buildRegExp(m_strToHighLight);
}

