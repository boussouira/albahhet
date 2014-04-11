#include "supportdialog.h"
#include "ui_supportdialog.h"
#include "networkrequest.h"
#include "common.h"

#include <qmessagebox.h>
#include <qdebug.h>
#include <qdom.h>
#include <qsettings.h>
#include <qcryptographichash.h>

#define DEFAULT_BASE_URL "https://dl.dropbox.com/s/mjjxktc3jvnkfq7/albahhet-messages.xml?dl=1"

SupportDialog::SupportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SupportDialog)
{
    ui->setupUi(this);

    m_check = true;
    m_showMaximized = false;

    QSettings settings;
    m_baseUpdateUrl = settings.value("SupportDialog/baseUrl",
                                     DEFAULT_BASE_URL).toString();

    connect(ui->webView, SIGNAL(loadFinished(bool)), SLOT(pageLoadDone(bool)));
}


void SupportDialog::startCheck()
{
    if(m_check)
        startRequest(QUrl(m_baseUpdateUrl));
}

void SupportDialog::startRequest(QUrl url)
{
    m_reply = m_qnam.get(NetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), SLOT(httpFinished()));
}

void SupportDialog::httpFinished()
{
    QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error()) {
        errorString = m_reply->errorString();
        m_hasError = true;

        qDebug() << "SupportDialog::httpFinished" << "Error:" << errorString
                 <<"Code" << m_reply->error();
        emit checkFinished();
    } else if (!redirectionTarget.isNull()) {
//        qDebug() << "SupportDialog::httpFinished" << "Redirect to"
//                 << redirectionTarget.toUrl().toString();

        startRequest(m_reply->url().resolved(redirectionTarget.toUrl()));
    } else {
        QString replayText = QString::fromUtf8(m_reply->readAll());

        if(replayText.size() > 10) {
//            qDebug() << "SupportDialog::httpFinished" << "Replay:" << replayText;
            parse(replayText);
        }

        m_hasError = false;
        emit checkFinished();
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void SupportDialog::parse(QString updateXML)
{
    QDomDocument doc;
    if(!doc.setContent(updateXML)) {
        qDebug() << "Parse error:" << updateXML ;
        return;
    }

    QDomElement root = doc.documentElement();
    QDomElement e = root.firstChildElement();

    QSettings settings;

    while(!e.isNull()) {
        QString mid = e.attribute("id");
        QString link = e.attribute("url");
        QString wTitle = e.attribute("title", windowTitle());

        if(mid.size() && (mid.compare("always", Qt::CaseInsensitive)==0
                || !settings.contains(QString("Messages/%1").arg(mid)))) {

            if(e.hasAttribute("update-url"))
                settings.setValue("Update/baseUrl", e.attribute("update-url"));

            if(e.hasAttribute("support-url"))
                settings.setValue("SupportDialog/baseUrl", e.attribute("support-url"));

            if(e.hasAttribute("user-agent"))
                settings.setValue("NetworkRequest/userAgent", e.attribute("user-agent"));

            if (e.hasAttribute("ignore")) {
                e = e.nextSiblingElement();
                continue;
            }

            m_mid = mid;

            if(e.hasAttribute("w") && e.hasAttribute("h"))
                resize(e.attribute("w").toInt(),
                       e.attribute("h").toInt());

            m_showMaximized = (e.hasAttribute("maximized")
                               && e.attribute("maximized").compare("true", Qt::CaseInsensitive)==0);

            setWindowTitle(wTitle);

            ui->webView->setUrl(link);

            break;
        }

        e = e.nextSiblingElement();
    }
}

void SupportDialog::pageLoadDone(bool success)
{
    if(success) {
        if(m_mid.size()) {
            QSettings settings;
            settings.beginGroup("Messages");
            settings.setValue(m_mid, true);
        }

        if(m_showMaximized) {
            setWindowState(windowState() | Qt::WindowMaximized);
        }

        disconnect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadDone(bool)));
        exec();
    }

    m_check = false;
}

SupportDialog::~SupportDialog()
{
    delete ui;
}

