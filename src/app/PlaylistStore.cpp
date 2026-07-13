#include "app/PlaylistStore.h"

#include "app/AppStorage.h"
#include "network/OnlineSongId.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QUuid>

namespace {

void normalizeTrackRef(PlaylistTrackRef& track)
{
    track.songId = OnlineSongId::normalizeLegacySongId(track.songId);
    if (track.sourceId.isEmpty()) {
        QString src;
        if (OnlineSongId::parse(track.songId, &src, nullptr)) {
            track.sourceId = src;
        }
    }
}

PlaylistTrackRef trackFromJson(const QJsonObject& obj)
{
    PlaylistTrackRef track;
    track.songId = obj.value(QStringLiteral("songId")).toString();
    track.sourceId = obj.value(QStringLiteral("sourceId")).toString();
    track.title = obj.value(QStringLiteral("title")).toString();
    track.artist = obj.value(QStringLiteral("artist")).toString();
    track.album = obj.value(QStringLiteral("album")).toString();
    track.detailUrl = obj.value(QStringLiteral("detailUrl")).toString();
    track.streamUrl = obj.value(QStringLiteral("streamUrl")).toString();
    track.coverUrl = obj.value(QStringLiteral("coverUrl")).toString();
    track.localPath = obj.value(QStringLiteral("localPath")).toString();
    normalizeTrackRef(track);
    return track;
}

QJsonObject trackToJson(const PlaylistTrackRef& track)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("songId"), track.songId);
    obj.insert(QStringLiteral("sourceId"), track.sourceId);
    obj.insert(QStringLiteral("title"), track.title);
    obj.insert(QStringLiteral("artist"), track.artist);
    obj.insert(QStringLiteral("album"), track.album);
    if (!track.detailUrl.isEmpty()) {
        obj.insert(QStringLiteral("detailUrl"), track.detailUrl);
    }
    obj.insert(QStringLiteral("streamUrl"), track.streamUrl);
    obj.insert(QStringLiteral("coverUrl"), track.coverUrl);
    obj.insert(QStringLiteral("localPath"), track.localPath);
    return obj;
}

} // namespace

PlaylistStore::PlaylistStore(QObject* parent)
    : QObject(parent)
{
    ensureDefaultPlaylist();
    load();
}

const QVector<PlaylistInfo>& PlaylistStore::playlists() const
{
    return m_playlists;
}

QString PlaylistStore::storagePath() const
{
    const QString dir = AppStorage::appDataDir();
    QDir().mkpath(dir);
    return dir + QStringLiteral("/playlists.json");
}

void PlaylistStore::ensureDefaultPlaylist()
{
    if (indexOfPlaylist(QStringLiteral("liked")) >= 0) {
        return;
    }

    PlaylistInfo liked;
    liked.id = QStringLiteral("liked");
    liked.name = QStringLiteral("我喜欢的音乐");
    liked.builtin = true;
    m_playlists.prepend(liked);
}

bool PlaylistStore::load()
{
    const QString path = storagePath();
    QFile file(path);
    if (!file.exists()) {
        ensureDefaultPlaylist();
        save();
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonArray playlistArray = root.value(QStringLiteral("playlists")).toArray();

    QVector<PlaylistInfo> loaded;
    loaded.reserve(playlistArray.size() + 1);

    for (const QJsonValue& value : playlistArray) {
        const QJsonObject obj = value.toObject();
        PlaylistInfo info;
        info.id = obj.value(QStringLiteral("id")).toString();
        info.name = obj.value(QStringLiteral("name")).toString();
        info.builtin = obj.value(QStringLiteral("builtin")).toBool(false);

        const QJsonArray tracks = obj.value(QStringLiteral("tracks")).toArray();
        info.tracks.reserve(tracks.size());
        for (const QJsonValue& trackValue : tracks) {
            info.tracks.append(trackFromJson(trackValue.toObject()));
        }
        loaded.append(info);
    }

    m_playlists = std::move(loaded);
    ensureDefaultPlaylist();
    emit playlistsChanged();
    return true;
}

bool PlaylistStore::save() const
{
    QJsonArray playlistArray;
    for (const PlaylistInfo& info : m_playlists) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), info.id);
        obj.insert(QStringLiteral("name"), info.name);
        obj.insert(QStringLiteral("builtin"), info.builtin);

        QJsonArray tracks;
        for (const PlaylistTrackRef& track : info.tracks) {
            tracks.append(trackToJson(track));
        }
        obj.insert(QStringLiteral("tracks"), tracks);
        playlistArray.append(obj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("playlists"), playlistArray);

    QFile file(storagePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

QString PlaylistStore::createPlaylist(const QString& name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    PlaylistInfo info;
    info.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    info.name = trimmed;
    info.builtin = false;
    m_playlists.append(info);
    save();
    emit playlistsChanged();
    return info.id;
}

bool PlaylistStore::deletePlaylist(const QString& id)
{
    const int idx = indexOfPlaylist(id);
    if (idx < 0) {
        return false;
    }
    if (m_playlists.at(idx).builtin) {
        return false;
    }

    m_playlists.removeAt(idx);
    save();
    emit playlistsChanged();
    return true;
}

bool PlaylistStore::addTrack(const QString& playlistId, const PlaylistTrackRef& track)
{
    PlaylistTrackRef normalized = track;
    normalizeTrackRef(normalized);
    if (normalized.songId.isEmpty()) {
        return false;
    }

    const int idx = indexOfPlaylist(playlistId);
    if (idx < 0) {
        return false;
    }

    PlaylistInfo& playlist = m_playlists[idx];
    for (const PlaylistTrackRef& existing : playlist.tracks) {
        if (existing.songId == normalized.songId) {
            return false;
        }
    }

    playlist.tracks.append(normalized);
    save();
    emit playlistsChanged();
    return true;
}

PlaylistAddResult PlaylistStore::addTracks(const QString& playlistId,
                                           const QVector<PlaylistTrackRef>& tracks)
{
    PlaylistAddResult result;
    const int idx = indexOfPlaylist(playlistId);
    if (idx < 0) {
        result.invalid = tracks.size();
        return result;
    }

    PlaylistInfo& playlist = m_playlists[idx];
    QSet<QString> existingIds;
    existingIds.reserve(playlist.tracks.size() + tracks.size());
    for (const PlaylistTrackRef& existing : playlist.tracks) {
        existingIds.insert(existing.songId);
    }

    for (PlaylistTrackRef track : tracks) {
        normalizeTrackRef(track);
        if (track.songId.isEmpty()) {
            ++result.invalid;
            continue;
        }
        if (existingIds.contains(track.songId)) {
            ++result.duplicate;
            continue;
        }
        playlist.tracks.append(track);
        existingIds.insert(track.songId);
        ++result.added;
    }

    if (result.added > 0) {
        save();
        emit playlistsChanged();
    }
    return result;
}

bool PlaylistStore::removeTrack(const QString& playlistId, const QString& songId)
{
    const int idx = indexOfPlaylist(playlistId);
    if (idx < 0) {
        return false;
    }

    PlaylistInfo& playlist = m_playlists[idx];
    for (int i = 0; i < playlist.tracks.size(); ++i) {
        if (playlist.tracks.at(i).songId == songId) {
            playlist.tracks.removeAt(i);
            save();
            emit playlistsChanged();
            return true;
        }
    }
    return false;
}

bool PlaylistStore::containsTrack(const QString& playlistId, const QString& songId) const
{
    const PlaylistInfo* playlist = playlistById(playlistId);
    if (!playlist) {
        return false;
    }
    for (const PlaylistTrackRef& track : playlist->tracks) {
        if (track.songId == songId) {
            return true;
        }
    }
    return false;
}

bool PlaylistStore::updateTrackStreamUrl(const QString& songId,
                                         const QString& streamUrl,
                                         const QString& coverUrl)
{
    if (songId.isEmpty() || streamUrl.isEmpty()) {
        return false;
    }

    bool changed = false;
    for (PlaylistInfo& playlist : m_playlists) {
        for (PlaylistTrackRef& track : playlist.tracks) {
            if (track.songId == songId) {
                track.streamUrl = streamUrl;
                if (!coverUrl.isEmpty()) {
                    track.coverUrl = coverUrl;
                }
                changed = true;
            }
        }
    }

    if (changed) {
        save();
        emit playlistsChanged();
    }
    return changed;
}

const PlaylistInfo* PlaylistStore::playlistById(const QString& id) const
{
    const int idx = indexOfPlaylist(id);
    if (idx < 0) {
        return nullptr;
    }
    return &m_playlists.at(idx);
}

int PlaylistStore::trackCount(const QString& playlistId) const
{
    const PlaylistInfo* playlist = playlistById(playlistId);
    return playlist ? playlist->tracks.size() : 0;
}

int PlaylistStore::indexOfPlaylist(const QString& id) const
{
    for (int i = 0; i < m_playlists.size(); ++i) {
        if (m_playlists.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QString PlaylistStore::localSongId(const QString& filePath)
{
    const QString absolute = QFileInfo(filePath).absoluteFilePath();
    return QStringLiteral("local:") + absolute;
}

bool PlaylistStore::isLocalSongId(const QString& songId)
{
    return songId.startsWith(QStringLiteral("local:"));
}
