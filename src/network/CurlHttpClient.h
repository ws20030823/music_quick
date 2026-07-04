#pragma once

#include <QByteArray>
#include <QPair>
#include <QString>
#include <QUrl>

struct HttpResponse {
    int statusCode = 0;
    QByteArray body;
    QString error;

    bool ok() const { return error.isEmpty() && statusCode >= 200 && statusCode < 300; }
};

class CurlHttpClient
{
public:
    using HeaderList = QList<QPair<QByteArray, QByteArray>>;

    static bool isAvailable();

    static HttpResponse get(const QUrl& url, const HeaderList& headers = {});
    static HttpResponse post(const QUrl& url,
                             const QByteArray& body,
                             const HeaderList& headers = {});
};
