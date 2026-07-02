#include "network/MyFreeMp3Client.h"

#include "network/MyFreeMp3Parser.h"

#include <QMetaType>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

namespace {

constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicQuick/1.0";
constexpr auto kBaseUrl = "https://myfreemp3online.com";

} // namespace

MyFreeMp3Client::MyFreeMp3Client(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<OnlineTrack>("OnlineTrack");
    qRegisterMetaType<SearchPageResult>("SearchPageResult");
}

QNetworkRequest MyFreeMp3Client::buildRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
    request.setTransferTimeout(15000);
    return request;
}

void MyFreeMp3Client::search(const QString& keyword, int page)
{
    QUrl url(QString::fromLatin1(kBaseUrl) + QStringLiteral("/search.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), keyword.trimmed());
    if (page > 1) {
        query.addQueryItem(QStringLiteral("page"), QString::number(page));
    }
    url.setQuery(query);

    QNetworkReply* reply = m_networkManager.get(buildRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, keyword]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit searchFailed(reply->errorString());
            return;
        }
        emit searchCompleted(parseSearchResultsHtml(QString::fromUtf8(reply->readAll())), keyword);
    });
}

void MyFreeMp3Client::resolveStreamUrl(const QString& songId)
{
    const QUrl url(QString::fromLatin1(kBaseUrl)
                 + QStringLiteral("/song/%1.html").arg(songId));

    QNetworkReply* reply = m_networkManager.get(buildRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, songId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit streamUrlFailed(songId, reply->errorString());
            return;
        }
        OnlineTrack track = parseSongDetailHtml(QString::fromUtf8(reply->readAll()), songId);
        if (track.streamUrl.isEmpty()) {
            emit streamUrlFailed(songId, QStringLiteral("未找到播放地址"));
            return;
        }
        emit streamUrlResolved(track);
    });
}
