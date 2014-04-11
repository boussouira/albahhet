#ifndef SUPPORTDIALOG_H
#define SUPPORTDIALOG_H

#include <QDialog>
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qurl.h>

namespace Ui {
class SupportDialog;
}

class SupportDialog : public QDialog
{
    Q_OBJECT

public:
    SupportDialog(QWidget *parent = 0);
    ~SupportDialog();

public slots:
    void startCheck();

private slots:
    void startRequest(QUrl url);
    void httpFinished();
    void parse(QString updateXML);
    void pageLoadDone(bool success);

signals:
    void checkFinished();

protected:
    QString m_baseUpdateUrl;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_reply;
    QString m_mid;
    QString errorString;
    bool m_hasError;
    bool m_check;
    bool m_showMaximized;
    Ui::SupportDialog *ui;
};

#endif // SUPPORTDIALOG_H
