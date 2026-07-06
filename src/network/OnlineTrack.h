#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>

// =============================================================================
// OnlineTrack.h — MyFreeMp3 在线曲目 DTO（network 层）
// =============================================================================
// 搜索列表只含 songId + 标题；点击后请求详情页补全 streamUrl / coverUrl。
// =============================================================================

struct OnlineTrack {
    QString sourceId; // e.g. "myfreemp3", "gequbao"
    QString songId;
    QString displayTitle;
    QString artist;
    QString title;
    QString detailUrl;
    QString streamUrl;
    QString coverUrl;
    QString lyrics;
};

struct SearchPageResult {
    QVector<OnlineTrack> tracks;
    int currentPage = 1;
    int totalPages = 1;
    bool hasPrevious = false;
    bool hasNext = false;
};

Q_DECLARE_METATYPE(OnlineTrack)
Q_DECLARE_METATYPE(SearchPageResult)
