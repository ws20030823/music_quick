#pragma once

#include "network/OnlineTrack.h"
#include "network/StreamFetchOptions.h"

#include <QObject>
#include <QString>
#include <QUrl>

// =============================================================================
// MusicSourceClient — 抽象在线音乐资源站客户端
// =============================================================================

class MusicSourceClient : public QObject
{
    Q_OBJECT

public:
    explicit MusicSourceClient(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    ~MusicSourceClient() override = default;

    virtual QString sourceId() const = 0;
    virtual QString displayName() const = 0;
    virtual StreamFetchOptions streamFetchOptions() const = 0;

    virtual void search(const QString& keyword, int page = 1) = 0;
    virtual void resolveStreamUrl(const QString& trackId,
                                  const QUrl& detailPageUrl = QUrl(),
                                  const QString& title = {},
                                  const QString& artist = {}) = 0;
    virtual void cancelResolveStreamUrl() = 0;

signals:
    void searchCompleted(const SearchPageResult& result, const QString& keyword);
    void searchFailed(const QString& message);
    void streamUrlResolved(const OnlineTrack& track);
    void streamUrlFailed(const QString& songId, const QString& message);
};
