#include "networkrequest.h"

NetworkRequest::NetworkRequest(const QUrl &url) :
    QNetworkRequest(url)
{
    setRawHeader("User-Agent", "albahhet.sourceforge.net");
}

