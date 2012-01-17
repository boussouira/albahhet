#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <qobject.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qurl.h>

class UpdateInfo {
public:
    UpdateInfo():revision(0), version(0) {}

    int revision;
    int version;
    QString versionStr;
    QString downloadLink;
    QString changelog;
};

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    UpdateChecker(QObject *parent = 0);
    ~UpdateChecker();

    UpdateInfo *result();

    QString errorString;
    bool hasError;
    bool autoCheck;

public slots:
    void startCheck(bool autoUpdateCheck=false);

private slots:
    void startRequest(QUrl url);
    void httpFinished();
    void updateError(QNetworkReply::NetworkError );
    void httpReadyRead();
    void parse(QString updateXML);

signals:
    void checkFinished();

protected:
    QString m_replayText;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_reply;
    UpdateInfo *m_result;
    QList<UpdateInfo*> m_results;
};

#endif // UPDATECHECKER_H
