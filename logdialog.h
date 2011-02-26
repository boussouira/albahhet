#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>

namespace Ui {
    class LogDialog;
}

class QFileSystemWatcher;

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    LogDialog(QWidget *parent = 0);
    ~LogDialog();

protected slots:
    void fileChanged(const QString & path);
    void clearLog();

private:
    Ui::LogDialog *ui;
    QFileSystemWatcher *m_watcher;
    QString m_logPath;
};

#endif // LOGDIALOG_H
