#pragma once

#include "network/OnlineTrack.h"

#include <optional>
#include <QString>

// FeaturedPlaylistCacheStore — 精选歌单搜索结果磁盘缓存（按 playlistId + page）
class FeaturedPlaylistCacheStore final
{
public:
    static std::optional<SearchPageResult> load(const QString& playlistId, int page);
    static void save(const QString& playlistId, int page, const SearchPageResult& result);
    static bool exists(const QString& playlistId, int page);

private:
    static QString cacheDirectory();
    static QString cacheFilePath(const QString& playlistId, int page);
};
