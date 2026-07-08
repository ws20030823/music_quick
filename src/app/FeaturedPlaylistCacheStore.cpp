#include "app/FeaturedPlaylistCacheStore.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QUrl>

namespace {

QJsonObject trackToJson(const OnlineTrack& track)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("sourceId"), track.sourceId);
    obj.insert(QStringLiteral("songId"), track.songId);
    obj.insert(QStringLiteral("displayTitle"), track.displayTitle);
    obj.insert(QStringLiteral("artist"), track.artist);
    obj.insert(QStringLiteral("title"), track.title);
    obj.insert(QStringLiteral("detailUrl"), track.detailUrl);
    obj.insert(QStringLiteral("streamUrl"), track.streamUrl);
    obj.insert(QStringLiteral("coverUrl"), track.coverUrl);
    obj.insert(QStringLiteral("lyrics"), track.lyrics);
    return obj;
}

OnlineTrack trackFromJson(const QJsonObject& obj)
{
    OnlineTrack track;
    track.sourceId = obj.value(QStringLiteral("sourceId")).toString();
    track.songId = obj.value(QStringLiteral("songId")).toString();
    track.displayTitle = obj.value(QStringLiteral("displayTitle")).toString();
    track.artist = obj.value(QStringLiteral("artist")).toString();
    track.title = obj.value(QStringLiteral("title")).toString();
    track.detailUrl = obj.value(QStringLiteral("detailUrl")).toString();
    track.streamUrl = obj.value(QStringLiteral("streamUrl")).toString();
    track.coverUrl = obj.value(QStringLiteral("coverUrl")).toString();
    track.lyrics = obj.value(QStringLiteral("lyrics")).toString();
    return track;
}

} // namespace

QString FeaturedPlaylistCacheStore::cacheDirectory()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/featured_playlists");
    QDir().mkpath(dir);
    return dir;
}

QString FeaturedPlaylistCacheStore::cacheFilePath(const QString& playlistId, int page)
{
    const QString safeId = QString::fromUtf8(
        QUrl::toPercentEncoding(playlistId.toUtf8(), "", "/\\:?*\"<>|"));
    return cacheDirectory() + QLatin1Char('/') + safeId + QStringLiteral("_p") + QString::number(page)
        + QStringLiteral(".json");
}

bool FeaturedPlaylistCacheStore::exists(const QString& playlistId, int page)
{
    return QFileInfo::exists(cacheFilePath(playlistId, page));
}

std::optional<SearchPageResult> FeaturedPlaylistCacheStore::load(const QString& playlistId, int page)
{
    QFile file(cacheFilePath(playlistId, page));
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return std::nullopt;
    }

    const QJsonObject root = document.object();
    SearchPageResult result;
    result.currentPage = root.value(QStringLiteral("currentPage")).toInt(1);
    result.totalPages = root.value(QStringLiteral("totalPages")).toInt(1);
    result.hasPrevious = root.value(QStringLiteral("hasPrevious")).toBool(false);
    result.hasNext = root.value(QStringLiteral("hasNext")).toBool(false);

    const QJsonArray tracks = root.value(QStringLiteral("tracks")).toArray();
    result.tracks.reserve(tracks.size());
    for (const QJsonValue& value : tracks) {
        if (!value.isObject()) {
            continue;
        }
        result.tracks.append(trackFromJson(value.toObject()));
    }

    if (result.tracks.isEmpty()) {
        return std::nullopt;
    }

    return result;
}

void FeaturedPlaylistCacheStore::save(const QString& playlistId,
                                      int page,
                                      const SearchPageResult& result)
{
    QJsonArray tracks;
    for (const OnlineTrack& track : result.tracks) {
        tracks.append(trackToJson(track));
    }

    QJsonObject root;
    root.insert(QStringLiteral("currentPage"), result.currentPage);
    root.insert(QStringLiteral("totalPages"), result.totalPages);
    root.insert(QStringLiteral("hasPrevious"), result.hasPrevious);
    root.insert(QStringLiteral("hasNext"), result.hasNext);
    root.insert(QStringLiteral("tracks"), tracks);

    QFile file(cacheFilePath(playlistId, page));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
}
