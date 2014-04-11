#include "updatechecker.h"
#include "networkrequest.h"

#include <qmessagebox.h>
#include <qdebug.h>
#include <qdom.h>
#include <QSettings>

#define DEFAULT_BASE_URL "http://albahhet.sourceforge.net/cms/index.php/updater/check/{GIT_REVISION}"

UpdateChecker::UpdateChecker(QObject *parent) :
    QObject(parent)
{
    hasError = false;
    autoCheck = false;

    QSettings settings;
    m_baseUpdateUrl = settings.value("Update/baseUrl",
                                     DEFAULT_BASE_URL).toString();
}

UpdateChecker::~UpdateChecker()
{
    qDeleteAll(m_results);
    m_results.clear();
}

void UpdateChecker::startCheck(bool autoUpdateCheck)
{
    m_result = 0;
    autoCheck = autoUpdateCheck;

#ifdef GITCHANGENUMBER
     QString url = QString(m_baseUpdateUrl).replace("{GIT_REVISION}", GITCHANGENUMBER);
    qDebug() << "Check:" << url;

    startRequest(QUrl(url));
#else
    qWarning("Can't check for update without Git change number");
#endif
}

UpdateInfo *UpdateChecker::result()
{
    return m_result;
}

void UpdateChecker::startRequest(QUrl url)
{
    m_reply = m_qnam.get(NetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), SLOT(httpFinished()));
}

void UpdateChecker::httpFinished()
{
    QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error()) {
        errorString = m_reply->errorString();
        hasError = true;

        emit checkFinished();
    } else if (!redirectionTarget.isNull()) {
        qDebug() << "UpdateChecker::httpFinished" << "Redirect to"
                 << redirectionTarget.toUrl().toString();

        startRequest(m_reply->url().resolved(redirectionTarget.toUrl()));
    } else {
        QString replayText = QString::fromUtf8(m_reply->readAll());

        if(replayText.size() > 10)
            parse(replayText);

        hasError = false;
        emit checkFinished();
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void UpdateChecker::parse(QString updateXML)
{
    QDomDocument doc;
    if(!doc.setContent(updateXML)) {
        qDebug() << "Parse error:" << updateXML ;
        m_result = 0;
        return;
    }

    m_result = new UpdateInfo();
    QDomElement root = doc.documentElement();

    QDomElement e = root.firstChildElement();
    while(!e.isNull()) {
        if(e.nodeName() == "revision") {
            m_result->revision = e.text().toInt();
        } else if(e.nodeName() == "version") {
            m_result->versionStr = e.text();
            m_result->version = e.attribute("num").toInt();
        } else if(e.nodeName() == "download") {
            m_result->downloadLink = e.text();
        } else if(e.nodeName() == "changelog") {
            m_result->changelog = e.text();
        }

        e = e.nextSiblingElement();
    }

    m_results.append(m_result);
#ifdef GITCHANGENUMBER
    if(m_result->revision <= QString(GITCHANGENUMBER).toInt())
        m_result = 0;
#endif
}
