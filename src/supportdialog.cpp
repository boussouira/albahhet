#include "supportdialog.h"
#include "ui_supportdialog.h"
#include "networkrequest.h"

#include <qmessagebox.h>
#include <qdebug.h>
#include <qdom.h>
#include <QSettings>

SupportDialog::SupportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SupportDialog)
{
    ui->setupUi(this);

    m_check = true;

    connect(ui->webView, SIGNAL(loadFinished(bool)), SLOT(pageLoadDone(bool)));
}


void SupportDialog::startCheck()
{
    if(m_check)
        startRequest(QUrl("https://dl.dropbox.com/s/mjjxktc3jvnkfq7/albahhet-messages.xml?dl=1"));
}

void SupportDialog::startRequest(QUrl url)
{
    m_reply = m_qnam.get(NetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), SLOT(httpFinished()));
    connect(m_reply, SIGNAL(readyRead()), SLOT(httpReadyRead()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            SLOT(updateError(QNetworkReply::NetworkError)));
}

void SupportDialog::httpFinished()
{
    QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error()) {
        errorString = m_reply->errorString();
        m_hasError = true;

        emit checkFinished();
    } else if (!redirectionTarget.isNull()) {
            QUrl url = url.resolved(redirectionTarget.toUrl());
            m_reply->deleteLater();
            m_replayText.clear();

            startRequest(url);
            return;
    } else {
        if(m_replayText.size() > 10)
            parse(m_replayText);

        m_hasError = false;
        emit checkFinished();
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void SupportDialog::updateError(QNetworkReply::NetworkError)
{
    errorString = m_reply->errorString();
    m_hasError = true;
}

void SupportDialog::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    m_replayText.append(QString::fromUtf8(m_reply->readAll()));
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
    settings.beginGroup("Messages");

    while(!e.isNull()) {
        QString mid = e.attribute("id");
        QString link = e.attribute("url");
        QString wTitle = e.attribute("title", windowTitle());

        if(mid.size() && link.size() && !settings.contains(mid)) {
            m_mid = mid;
            ui->webView->setUrl(link);

            if(e.hasAttribute("w") && e.hasAttribute("h"))
                ui->webView->resize(e.attribute("w").toInt(),
                                    e.attribute("h").toInt());

            setWindowTitle(wTitle);

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

        show();
    }

    m_check = false;
}

SupportDialog::~SupportDialog()
{
    delete ui;
}

