#ifndef NETWORKREQUEST_H
#define NETWORKREQUEST_H

#include <qnetworkrequest.h>

class NetworkRequest : public QNetworkRequest
{
public:
    NetworkRequest(const QUrl &url = QUrl());

signals:

public slots:

};

#endif // NETWORKREQUEST_H
