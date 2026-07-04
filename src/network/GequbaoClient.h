#pragma once

#include "network/CurlHttpClient.h"
#include "network/GequbaoParser.h"
#include "network/MusicSourceClient.h"
#include "network/StreamFetchOptions.h"

#include <functional>

#include <QNetworkAccessManager>
#include <QPointer>
#include <QUrl>

class QNetworkReply;

class GequbaoClient final : public MusicSourceClient
{
    Q_OBJECT

public:
    explicit GequbaoClient(QObject* parent = nullptr);

    QString sourceId() const override;
    QString displayName() const override;
    StreamFetchOptions streamFetchOptions() const override;

    void search(const QString& keyword, int page = 1) override;
    void resolveStreamUrl(const QString& trackId,
                          const QUrl& detailPageUrl = QUrl(),
                          const QString& title = {},
                          const QString& artist = {}) override;
    void cancelResolveStreamUrl() override;

private:
    static QString userAgent();
    static CurlHttpClient::HeaderList documentHeaders(const QString& referer);
    static CurlHttpClient::HeaderList apiHeaders(const QString& referer);
    static QUrl buildSearchUrl(const QString& keyword, int page);

    void performSearch(const QString& keyword, int page);
    void handleSearchResponse(const HttpResponse& response, const QString& keyword);
    void handleDetailResponse(const QString& trackId,
                              const QString& bareTrackId,
                              const HttpResponse& response);
    void handlePlayUrlResponse(const QString& trackId,
                               const QString& bareTrackId,
                               const GequbaoMusicDetail& detail,
                               const HttpResponse& response);

    void fetchMusicDetail(const QString& trackId, const QString& bareTrackId, const QUrl& detailUrl);
    void fetchPlayUrl(const QString& trackId,
                      const QString& bareTrackId,
                      const GequbaoMusicDetail& detail);

#if defined(Q_OS_WIN)
    void scheduleCurl(const std::function<HttpResponse()>& task,
                      const std::function<void(const HttpResponse&)>& callback);
#else
    static HttpResponse responseFromReply(QNetworkReply* reply);
    static QNetworkRequest buildNetworkRequest(const QUrl& url, const QString& referer);
#endif

    QNetworkAccessManager m_networkManager;
    QPointer<QNetworkReply> m_activeResolveReply;
};
