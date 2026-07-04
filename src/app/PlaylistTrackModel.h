#pragma once

#include <QAbstractListModel>
#include <QSet>
#include <QVector>

#include "app/PlaylistStore.h"
#include "media/TrackMetadata.h"

struct PlaylistTrackEntry {
    PlaylistTrackRef ref;
    TrackMetadata metadata;
    bool isPlaying = false;
    bool isSelected = false;
};

class PlaylistTrackModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        FilePathRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        DurationRole,
        HasCoverRole,
        CoverRole,
        IsPlayingRole,
        RowIndexRole,
        SongIdRole,
        StreamUrlRole,
        IsLikedRole,
        IsSelectedRole,
    };

    explicit PlaylistTrackModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTracks(QVector<PlaylistTrackEntry> entries);
    const QVector<PlaylistTrackEntry>& entries() const;

    void setLikedSongIds(const QSet<QString>& songIds);
    void refreshLikedState(int row, bool liked);

    void setPlayingRow(int row);
    int playingRow() const;

    void setSelectedRow(int row);
    int selectedRow() const;

    QString songIdAt(int row) const;
    QString streamUrlAt(int row) const;

private:
    QVector<PlaylistTrackEntry> m_entries;
    int m_playingRow = -1;
    int m_selectedRow = -1;
    QSet<QString> m_likedSongIds;
};
