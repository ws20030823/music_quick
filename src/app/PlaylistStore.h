#pragma once

#include <QObject>
#include <QString>
#include <QVector>

struct PlaylistTrackRef {
    QString songId;
    QString sourceId;
    QString title;
    QString artist;
    QString album;
    QString detailUrl;
    QString streamUrl;
    QString coverUrl;
    QString localPath;
};

struct PlaylistInfo {
    QString id;
    QString name;
    bool builtin = false;
    QVector<PlaylistTrackRef> tracks;
};

class PlaylistStore final : public QObject
{
    Q_OBJECT

public:
    static constexpr const char* kLikedPlaylistId = "liked";

    explicit PlaylistStore(QObject* parent = nullptr);

    const QVector<PlaylistInfo>& playlists() const;
    bool load();
    bool save() const;

    QString createPlaylist(const QString& name);
    bool deletePlaylist(const QString& id);
    bool addTrack(const QString& playlistId, const PlaylistTrackRef& track);
    bool removeTrack(const QString& playlistId, const QString& songId);
    bool containsTrack(const QString& playlistId, const QString& songId) const;
    bool updateTrackStreamUrl(const QString& songId, const QString& streamUrl, const QString& coverUrl);
    const PlaylistInfo* playlistById(const QString& id) const;
    int trackCount(const QString& playlistId) const;

    static QString localSongId(const QString& filePath);
    static bool isLocalSongId(const QString& songId);

signals:
    void playlistsChanged();

private:
    void ensureDefaultPlaylist();
    int indexOfPlaylist(const QString& id) const;
    QString storagePath() const;

    QVector<PlaylistInfo> m_playlists;
};
