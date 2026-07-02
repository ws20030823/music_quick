#pragma once

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;

// =============================================================================
// OnlineStreamLoader — 带 Referer 预取在线音频到本地缓存
// =============================================================================
// MyFreeMp3 的 CDN 对无 Referer 的请求返回 403；Qt Multimedia / FFmpeg
// 无法为 setSource(QUrl) 附加自定义 HTTP 头，因此先下载再本地播放。
// =============================================================================

class OnlineStreamLoader final : public QObject
{
    Q_OBJECT

public:
    explicit OnlineStreamLoader(QObject* parent = nullptr);

    // 若缓存命中则立即 ready；否则带 Referer 下载到 CacheLocation/streams/
    void prefetch(const QUrl& streamUrl);

signals:
    void prefetchReady(const QString& localFilePath, const QUrl& originalUrl);
    void prefetchFailed(const QUrl& originalUrl, const QString& message);

private:
    QString cacheFilePathFor(const QUrl& streamUrl) const;

    QNetworkAccessManager* m_networkManager = nullptr;
};
