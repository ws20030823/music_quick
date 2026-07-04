#include "network/MyFreeMp3Client.h"



#include "network/MyFreeMp3Parser.h"

#include "network/OnlineSongId.h"



#include <QMetaType>

#include <QNetworkReply>

#include <QNetworkRequest>

#include <QUrlQuery>



namespace {



constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicQuick/1.0";

constexpr auto kBaseUrl = "https://myfreemp3online.com";

constexpr auto kSourceId = "myfreemp3";



QString bareTrackIdFromSongId(const QString& songId)

{

    QString src;

    QString trackId;

    if (OnlineSongId::parse(songId, &src, &trackId)) {

        return trackId;

    }

    return songId;

}



} // namespace



MyFreeMp3Client::MyFreeMp3Client(QObject* parent)

    : MusicSourceClient(parent)

{

    qRegisterMetaType<OnlineTrack>("OnlineTrack");

    qRegisterMetaType<SearchPageResult>("SearchPageResult");

}



QString MyFreeMp3Client::sourceId() const

{

    return QString::fromLatin1(kSourceId);

}



QString MyFreeMp3Client::displayName() const

{

    return QStringLiteral("MyFreeMp3");

}



StreamFetchOptions MyFreeMp3Client::streamFetchOptions() const

{

    StreamFetchOptions options;

    options.referer = QString::fromLatin1(kBaseUrl) + QLatin1Char('/');

    options.userAgent = QString::fromLatin1(kUserAgent);

    return options;

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



void MyFreeMp3Client::cancelResolveStreamUrl()

{

    if (!m_activeResolveReply) {

        return;

    }



    QNetworkReply* reply = m_activeResolveReply;

    m_activeResolveReply = nullptr;

    reply->abort();

    reply->deleteLater();

}



void MyFreeMp3Client::resolveStreamUrl(const QString& trackId,
                                       const QUrl& detailPageUrl,
                                       const QString& title,
                                       const QString& artist)

{

    Q_UNUSED(detailPageUrl);
    Q_UNUSED(title);
    Q_UNUSED(artist);

    cancelResolveStreamUrl();



    const QString bareTrackId = bareTrackIdFromSongId(trackId);



    const QUrl url(QString::fromLatin1(kBaseUrl)

                 + QStringLiteral("/song/%1.html").arg(bareTrackId));



    QNetworkReply* reply = m_networkManager.get(buildRequest(url));

    m_activeResolveReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply, trackId]() {

        if (m_activeResolveReply == reply) {

            m_activeResolveReply = nullptr;

        }

        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {

            if (reply->error() == QNetworkReply::OperationCanceledError) {

                return;

            }

            emit streamUrlFailed(trackId, reply->errorString());

            return;

        }

        OnlineTrack track = parseSongDetailHtml(QString::fromUtf8(reply->readAll()), trackId);

        if (track.streamUrl.isEmpty()) {

            emit streamUrlFailed(trackId, QStringLiteral("未找到播放地址"));

            return;

        }

        emit streamUrlResolved(track);

    });

}

