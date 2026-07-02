#pragma once

#include "network/OnlineTrack.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>

// =============================================================================
// MyFreeMp3Client — 非同步 HTTP 客户端
// =============================================================================
// search(keyword, page)  → GET search.php
// resolveStreamUrl(id)  → GET song/{id}.html，解析 CDN 播放直链
// =============================================================================

class MyFreeMp3Client final : public QObject
{
    Q_OBJECT

public:
    explicit MyFreeMp3Client(QObject* parent = nullptr);

    void search(const QString& keyword, int page = 1);
    void resolveStreamUrl(const QString& songId);

signals:
    void searchCompleted(const SearchPageResult& result, const QString& keyword);
    void searchFailed(const QString& message);
    void streamUrlResolved(const OnlineTrack& track);
    void streamUrlFailed(const QString& songId, const QString& message);

private:
    static QNetworkRequest buildRequest(const QUrl& url);

    QNetworkAccessManager m_networkManager;
};
