#include "loghighlighter.h"

LogHighlighter::LogHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
}

void LogHighlighter::highlightBlock(const QString &text)
{
    if(text.contains("[DEBUG]")) {
        /*
        QTextCharFormat debugFormat;
        debugFormat.setForeground(QColor("#008800"));

        setFormat(0, text.size(), debugFormat);
        */
    } else if(text.contains("[WARNING]")) {
        QTextCharFormat  warrFormat;
        warrFormat.setForeground(QColor("#e90000"));

        setFormat(0, text.size(), warrFormat);
    } else if(text.contains("[CRITICAL]") ||
              text.contains("[FATAL]")) {
        QFont font;
        font.setBold(true);

        QTextCharFormat  errorFormat;
        errorFormat.setForeground(QColor("#e90000"));
        errorFormat.setFont(font);

        setFormat(0, text.size(), errorFormat);
    }
}
