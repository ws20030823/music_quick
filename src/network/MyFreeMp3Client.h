#pragma once



#include "network/MusicSourceClient.h"

#include "network/StreamFetchOptions.h"



#include <QNetworkAccessManager>

#include <QPointer>

#include <QUrl>



class QNetworkReply;



// =============================================================================

// MyFreeMp3Client — 非同步 HTTP 客户端

// =============================================================================

// search(keyword, page)  → GET search.php

// resolveStreamUrl(id)  → GET song/{id}.html，解析 CDN 播放直链

// =============================================================================



class MyFreeMp3Client final : public MusicSourceClient

{

    Q_OBJECT



public:

    explicit MyFreeMp3Client(QObject* parent = nullptr);



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

    static QNetworkRequest buildRequest(const QUrl& url);



    QNetworkAccessManager m_networkManager;

    QPointer<QNetworkReply> m_activeResolveReply;

};

