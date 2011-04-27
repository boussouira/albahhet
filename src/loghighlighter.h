#ifndef LOGHIGHLIGHTER_H
#define LOGHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class LogHighlighter :  public QSyntaxHighlighter
{
public:
    LogHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);
};

#endif // LOGHIGHLIGHTER_H
