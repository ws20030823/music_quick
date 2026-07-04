#include "network/GequbaoClient.h"

#include "network/GequbaoParser.h"
#include "network/OnlineSongId.h"

#include <QMetaType>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#if defined(Q_OS_WIN)
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#endif

namespace {

constexpr auto kBaseUrl = "https://www.gequbao.net";
constexpr auto kSourceId = "gequbao";

QPair<QByteArray, QByteArray> httpHeader(const char* name, const QByteArray& value)
{
    return qMakePair(QByteArray(name), value);
}

QPair<QByteArray, QByteArray> httpHeader(const char* name, const char* value)
{
    return qMakePair(QByteArray(name), QByteArray(value));
}

QString bareTrackIdFromSongId(const QString& songId)
{
    QString src;
    QString trackId;
    if (OnlineSongId::parse(songId, &src, &trackId)) {
        return trackId;
    }
    return songId;
}

OnlineTrack buildTrackFromDetail(const QString& trackId,
                                 const QString& bareTrackId,
                                 const GequbaoMusicDetail& detail,
                                 const QString& streamUrl)
{
    OnlineTrack track;
    track.sourceId = QString::fromLatin1(kSourceId);
    if (OnlineSongId::parse(trackId, nullptr, nullptr)) {
        track.songId = trackId;
    } else {
        track.songId = OnlineSongId::compose(track.sourceId, bareTrackId);
    }
    track.title = detail.title;
    track.artist = detail.artist;
    if (!detail.title.isEmpty() && !detail.artist.isEmpty()) {
        track.displayTitle = detail.title + QStringLiteral(" - ") + detail.artist;
    } else {
        track.displayTitle = detail.title.isEmpty() ? detail.artist : detail.title;
    }
    const QString musicId = detail.mp3Id.isEmpty() ? bareTrackId : detail.mp3Id;
    track.detailUrl = QString::fromLatin1(kBaseUrl) + QStringLiteral("/music/") + musicId;
    track.coverUrl = detail.coverUrl;
    track.streamUrl = streamUrl;
    return track;
}

} // namespace

GequbaoClient::GequbaoClient(QObject* parent)
    : MusicSourceClient(parent)
{
    qRegisterMetaType<OnlineTrack>("OnlineTrack");
    qRegisterMetaType<SearchPageResult>("SearchPageResult");
}

QString GequbaoClient::userAgent()
{
    return QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                          "(KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36");
}

QString GequbaoClient::sourceId() const
{
    return QString::fromLatin1(kSourceId);
}

QString GequbaoClient::displayName() const
{
    return QStringLiteral("歌曲宝");
}

StreamFetchOptions GequbaoClient::streamFetchOptions() const
{
    StreamFetchOptions options;
    options.referer = QString::fromLatin1(kBaseUrl) + QLatin1Char('/');
    options.userAgent = userAgent();
    return options;
}

CurlHttpClient::HeaderList GequbaoClient::documentHeaders(const QString& referer)
{
    CurlHttpClient::HeaderList headers;
    headers.append(httpHeader("User-Agent", userAgent().toUtf8()));
    headers.append(httpHeader("Referer", referer.toUtf8()));
    headers.append(httpHeader("Accept",
                              "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"));
    headers.append(httpHeader("Accept-Language", "zh-CN,zh;q=0.9"));
    return headers;
}

CurlHttpClient::HeaderList GequbaoClient::apiHeaders(const QString& referer)
{
    CurlHttpClient::HeaderList headers = documentHeaders(referer);
    headers.append(httpHeader("Content-Type", "application/x-www-form-urlencoded"));
    return headers;
}

QUrl GequbaoClient::buildSearchUrl(const QString& keyword, int page)
{
    QUrl url(QString::fromLatin1(kBaseUrl)
             + QStringLiteral("/s/")
             + QString::fromUtf8(QUrl::toPercentEncoding(keyword.trimmed().toUtf8(), "", "/")));
    if (page > 1) {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("page"), QString::number(page));
        url.setQuery(query);
    }
    return url;
}

#if !defined(Q_OS_WIN)
HttpResponse GequbaoClient::responseFromReply(QNetworkReply* reply)
{
    HttpResponse response;
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    response.body = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        response.error = QStringLiteral("%1 (HTTP %2)")
                             .arg(reply->errorString())
                             .arg(response.statusCode);
    } else if (response.statusCode >= 400) {
        response.error = QStringLiteral("HTTP %1").arg(response.statusCode);
    }
    return response;
}

QNetworkRequest GequbaoClient::buildNetworkRequest(const QUrl& url, const QString& referer)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent());
    request.setRawHeader("Referer", referer.toUtf8());
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(15000);
    return request;
}
#endif

#if defined(Q_OS_WIN)
void GequbaoClient::scheduleCurl(const std::function<HttpResponse()>& task,
                                 const std::function<void(const HttpResponse&)>& callback)
{
    if (!CurlHttpClient::isAvailable()) {
        callback({ .error = QStringLiteral("未找到 curl.exe，无法访问歌曲宝") });
        return;
    }

    auto* watcher = new QFutureWatcher<HttpResponse>(this);
    connect(watcher, &QFutureWatcher<HttpResponse>::finished, this, [watcher, callback]() {
        const HttpResponse response = watcher->result();
        watcher->deleteLater();
        callback(response);
    });
    watcher->setFuture(QtConcurrent::run(task));
}
#endif

void GequbaoClient::search(const QString& keyword, int page)
{
    const QString trimmedKeyword = keyword.trimmed();
    if (trimmedKeyword.isEmpty()) {
        emit searchFailed(QStringLiteral("请输入歌名或歌手"));
        return;
    }

    performSearch(trimmedKeyword, page);
}

void GequbaoClient::performSearch(const QString& keyword, int page)
{
    const QUrl url = buildSearchUrl(keyword, page);
    const QString referer = QString::fromLatin1(kBaseUrl) + QLatin1Char('/');

#if defined(Q_OS_WIN)
    const CurlHttpClient::HeaderList headers = documentHeaders(referer);
    scheduleCurl([url, headers]() { return CurlHttpClient::get(url, headers); },
                 [this, keyword](const HttpResponse& response) {
                     handleSearchResponse(response, keyword);
                 });
#else
    QNetworkReply* reply = m_networkManager.get(buildNetworkRequest(url, referer));
    connect(reply, &QNetworkReply::finished, this, [this, reply, keyword]() {
        reply->deleteLater();
        handleSearchResponse(responseFromReply(reply), keyword);
    });
#endif
}

void GequbaoClient::handleSearchResponse(const HttpResponse& response, const QString& keyword)
{
    if (!response.error.isEmpty()) {
        emit searchFailed(response.error);
        return;
    }

    const SearchPageResult result = parseGequbaoSearchHtml(QString::fromUtf8(response.body));
    if (result.tracks.isEmpty()) {
        emit searchFailed(QStringLiteral("无法解析搜索结果（收到 %1 字节，可能页面结构已变化）")
                              .arg(response.body.size()));
        return;
    }

    emit searchCompleted(result, keyword);
}

void GequbaoClient::cancelResolveStreamUrl()
{
    if (!m_activeResolveReply) {
        return;
    }

    QNetworkReply* reply = m_activeResolveReply;
    m_activeResolveReply = nullptr;
    reply->abort();
    reply->deleteLater();
}

void GequbaoClient::resolveStreamUrl(const QString& trackId,
                                     const QUrl& detailPageUrl,
                                     const QString& title,
                                     const QString& artist)
{
    cancelResolveStreamUrl();

    const QString bareTrackId = bareTrackIdFromSongId(trackId);
    QUrl detailUrl = detailPageUrl;

    if (detailUrl.isEmpty() && !title.isEmpty()) {
        QUrl url(QString::fromLatin1(kBaseUrl) + QStringLiteral("/search_music"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("song_id"), bareTrackId);
        query.addQueryItem(QStringLiteral("title"), title);
        query.addQueryItem(QStringLiteral("singer"), artist.isEmpty() ? title : artist);
        query.addQueryItem(QStringLiteral("kwd"), artist.isEmpty() ? title : artist);
        url.setQuery(query);
        detailUrl = url;
    } else if (detailUrl.isRelative()) {
        detailUrl = QUrl(QString::fromLatin1(kBaseUrl) + detailUrl.toString(QUrl::FullyEncoded));
    } else if (detailUrl.isEmpty()) {
        detailUrl = QUrl(QString::fromLatin1(kBaseUrl)
                         + QStringLiteral("/search_music?song_id=") + bareTrackId);
    }

    fetchMusicDetail(trackId, bareTrackId, detailUrl);
}

void GequbaoClient::handleDetailResponse(const QString& trackId,
                                         const QString& bareTrackId,
                                         const HttpResponse& response)
{
    if (!response.error.isEmpty()) {
        emit streamUrlFailed(trackId, response.error);
        return;
    }

    const GequbaoMusicDetail detail = parseGequbaoMusicDetailHtml(QString::fromUtf8(response.body));
    if (!detail.isValid()) {
        emit streamUrlFailed(trackId, QStringLiteral("未找到播放信息"));
        return;
    }

    fetchPlayUrl(trackId, bareTrackId, detail);
}

void GequbaoClient::handlePlayUrlResponse(const QString& trackId,
                                          const QString& bareTrackId,
                                          const GequbaoMusicDetail& detail,
                                          const HttpResponse& response)
{
    if (!response.error.isEmpty()) {
        emit streamUrlFailed(trackId, response.error);
        return;
    }

    const QString streamUrl = parseGequbaoPlayUrlJson(QString::fromUtf8(response.body));
    if (streamUrl.isEmpty()) {
        emit streamUrlFailed(trackId, QStringLiteral("未找到播放地址"));
        return;
    }

    emit streamUrlResolved(buildTrackFromDetail(trackId, bareTrackId, detail, streamUrl));
}

void GequbaoClient::fetchMusicDetail(const QString& trackId,
                                     const QString& bareTrackId,
                                     const QUrl& detailUrl)
{
    const QString referer = QString::fromLatin1(kBaseUrl) + QLatin1Char('/');

#if defined(Q_OS_WIN)
    const CurlHttpClient::HeaderList headers = documentHeaders(referer);
    scheduleCurl([detailUrl, headers]() { return CurlHttpClient::get(detailUrl, headers); },
                 [this, trackId, bareTrackId](const HttpResponse& response) {
                     handleDetailResponse(trackId, bareTrackId, response);
                 });
#else
    QNetworkReply* reply = m_networkManager.get(buildNetworkRequest(detailUrl, referer));
    m_activeResolveReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, trackId, bareTrackId]() {
        if (m_activeResolveReply == reply) {
            m_activeResolveReply = nullptr;
        }
        reply->deleteLater();

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            return;
        }

        handleDetailResponse(trackId, bareTrackId, responseFromReply(reply));
    });
#endif
}

void GequbaoClient::fetchPlayUrl(const QString& trackId,
                                 const QString& bareTrackId,
                                 const GequbaoMusicDetail& detail)
{
    const QUrl url(QString::fromLatin1(kBaseUrl) + QStringLiteral("/api/play-url"));
    const QString referer = QString::fromLatin1(kBaseUrl) + QStringLiteral("/music/")
                            + (detail.mp3Id.isEmpty() ? bareTrackId : detail.mp3Id);
    const QByteArray body = QByteArray("id=") + QUrl::toPercentEncoding(detail.playId);

#if defined(Q_OS_WIN)
    const CurlHttpClient::HeaderList headers = apiHeaders(referer);
    scheduleCurl([url, body, headers]() { return CurlHttpClient::post(url, body, headers); },
                 [this, trackId, bareTrackId, detail](const HttpResponse& response) {
                     handlePlayUrlResponse(trackId, bareTrackId, detail, response);
                 });
#else
    QNetworkRequest request = buildNetworkRequest(url, referer);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                       QStringLiteral("application/x-www-form-urlencoded"));

    QNetworkReply* reply = m_networkManager.post(request, body);
    m_activeResolveReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, trackId, bareTrackId, detail]() {
        if (m_activeResolveReply == reply) {
            m_activeResolveReply = nullptr;
        }
        reply->deleteLater();

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            return;
        }

        handlePlayUrlResponse(trackId, bareTrackId, detail, responseFromReply(reply));
    });
#endif
}
