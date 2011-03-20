#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>

namespace Ui {
    class LogDialog;
}

class QFileSystemWatcher;
class LogHighlighter;

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    LogDialog(QWidget *parent = 0);
    ~LogDialog();
    void stopWatching();
    void startWatching();

protected:
    void closeEvent(QCloseEvent *event);

protected slots:
    void fileChanged(const QString & path);
    void clearLog();
    void hideDialog();

private:
    Ui::LogDialog *ui;
    QFileSystemWatcher *m_watcher;
    QString m_logPath;
    LogHighlighter *m_highlighter;
};

#endif // LOGDIALOG_H
