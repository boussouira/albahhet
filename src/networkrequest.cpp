#include "networkrequest.h"
#include <qsettings.h>

NetworkRequest::NetworkRequest(const QUrl &url) :
    QNetworkRequest(url)
{
    QSettings settings;
    QString userAgent = settings.value("NetworkRequest/userAgent",
                                       "albahhet.sourceforge.net").toString();

    setRawHeader("User-Agent", userAgent.toUtf8());
}

