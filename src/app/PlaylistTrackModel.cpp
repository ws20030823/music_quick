#include "app/PlaylistTrackModel.h"

PlaylistTrackModel::PlaylistTrackModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int PlaylistTrackModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

QVariant PlaylistTrackModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }

    const PlaylistTrackEntry& entry = m_entries.at(index.row());
    switch (role) {
    case FilePathRole:
        return entry.ref.streamUrl;
    case TitleRole:
        return entry.metadata.title.isEmpty() ? entry.ref.title : entry.metadata.title;
    case ArtistRole:
        return entry.metadata.artist.isEmpty() ? entry.ref.artist : entry.metadata.artist;
    case AlbumRole:
        return entry.metadata.album.isEmpty() ? entry.ref.album : entry.metadata.album;
    case DurationRole:
        return entry.metadata.durationText;
    case HasCoverRole:
        return !entry.metadata.cover.isNull();
    case CoverRole:
        return entry.metadata.cover;
    case IsPlayingRole:
        return entry.isPlaying;
    case RowIndexRole:
        return index.row();
    case SongIdRole:
        return entry.ref.songId;
    case StreamUrlRole:
        return entry.ref.streamUrl;
    case IsLikedRole:
        return m_likedSongIds.contains(entry.ref.songId);
    case IsSelectedRole:
        return entry.isSelected;
    default:
        return {};
    }
}

QHash<int, QByteArray> PlaylistTrackModel::roleNames() const
{
    return {
        {FilePathRole, "filePath"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {AlbumRole, "album"},
        {DurationRole, "duration"},
        {HasCoverRole, "hasCover"},
        {CoverRole, "cover"},
        {IsPlayingRole, "isPlaying"},
        {RowIndexRole, "rowIndex"},
        {SongIdRole, "songId"},
        {StreamUrlRole, "streamUrl"},
        {IsLikedRole, "isLiked"},
        {IsSelectedRole, "isSelected"},
    };
}

void PlaylistTrackModel::setTracks(QVector<PlaylistTrackEntry> entries)
{
    beginResetModel();
    m_entries = std::move(entries);
    m_playingRow = -1;
    m_selectedRow = -1;
    endResetModel();
}

void PlaylistTrackModel::setLikedSongIds(const QSet<QString>& songIds)
{
    m_likedSongIds = songIds;
    if (m_entries.isEmpty()) {
        return;
    }
    emit dataChanged(index(0), index(m_entries.size() - 1), {IsLikedRole});
}

void PlaylistTrackModel::refreshLikedState(int row, bool liked)
{
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    const QString songId = m_entries.at(row).ref.songId;
    if (liked) {
        m_likedSongIds.insert(songId);
    } else {
        m_likedSongIds.remove(songId);
    }
    emit dataChanged(index(row), index(row), {IsLikedRole});
}

const QVector<PlaylistTrackEntry>& PlaylistTrackModel::entries() const
{
    return m_entries;
}

void PlaylistTrackModel::setPlayingRow(int row)
{
    if (m_playingRow == row) {
        return;
    }

    auto updateRow = [this](int r, bool playing) {
        if (r < 0 || r >= m_entries.size()) {
            return;
        }
        m_entries[r].isPlaying = playing;
        emit dataChanged(index(r), index(r), {IsPlayingRole});
    };

    updateRow(m_playingRow, false);
    m_playingRow = row;
    updateRow(m_playingRow, true);
}

int PlaylistTrackModel::playingRow() const
{
    return m_playingRow;
}

void PlaylistTrackModel::setSelectedRow(int row)
{
    if (m_selectedRow == row) {
        return;
    }

    auto updateRow = [this](int r, bool selected) {
        if (r < 0 || r >= m_entries.size()) {
            return;
        }
        m_entries[r].isSelected = selected;
        emit dataChanged(index(r), index(r), {IsSelectedRole});
    };

    updateRow(m_selectedRow, false);
    m_selectedRow = row;
    updateRow(m_selectedRow, true);
}

int PlaylistTrackModel::selectedRow() const
{
    return m_selectedRow;
}

QString PlaylistTrackModel::songIdAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }
    return m_entries.at(row).ref.songId;
}

QString PlaylistTrackModel::streamUrlAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }
    return m_entries.at(row).ref.streamUrl;
}
