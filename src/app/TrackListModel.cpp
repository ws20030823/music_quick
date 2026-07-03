#include "app/TrackListModel.h"

#include <QFileInfo>

namespace {

QString formatFileSize(qint64 bytes)
{
    if (bytes <= 0) {
        return QStringLiteral("--");
    }
    const double mb = bytes / (1024.0 * 1024.0);
    if (mb >= 1.0) {
        return QStringLiteral("%1M").arg(QString::number(mb, 'f', 1));
    }
    const double kb = bytes / 1024.0;
    return QStringLiteral("%1K").arg(QString::number(kb, 'f', 0));
}

} // namespace

TrackListModel::TrackListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TrackListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

QVariant TrackListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }

    const TrackEntry& entry = m_entries.at(index.row());
    switch (role) {
    case FilePathRole:
        return entry.filePath;
    case TitleRole:
        return entry.metadata.title;
    case ArtistRole:
        return entry.metadata.artist;
    case AlbumRole:
        return entry.metadata.album;
    case DurationRole:
        return entry.metadata.durationText;
    case HasCoverRole:
        return !entry.metadata.cover.isNull();
    case CoverRole:
        return entry.metadata.cover;
    case IsPlayingRole:
        return entry.isPlaying;
    case IsSelectedRole:
        return entry.isSelected;
    case IsLikedRole:
        return m_likedSongIds.contains(localSongIdAt(index.row()));
    case FileSizeRole:
        return formatFileSize(QFileInfo(entry.filePath).size());
    case RowIndexRole:
        return index.row();
    default:
        return {};
    }
}

QHash<int, QByteArray> TrackListModel::roleNames() const
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
        {IsSelectedRole, "isSelected"},
        {IsLikedRole, "isLiked"},
        {FileSizeRole, "fileSize"},
        {RowIndexRole, "rowIndex"},
    };
}

void TrackListModel::setTracks(QVector<TrackEntry> tracks)
{
    beginResetModel();
    m_entries = std::move(tracks);
    m_playingRow = -1;
    m_selectedRow = -1;
    endResetModel();
}

const QVector<TrackEntry>& TrackListModel::entries() const
{
    return m_entries;
}

void TrackListModel::setPlayingRow(int row)
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

int TrackListModel::playingRow() const
{
    return m_playingRow;
}

void TrackListModel::setSelectedRow(int row)
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

int TrackListModel::selectedRow() const
{
    return m_selectedRow;
}

void TrackListModel::setLikedSongIds(const QSet<QString>& songIds)
{
    m_likedSongIds = songIds;
    if (m_entries.isEmpty()) {
        return;
    }
    emit dataChanged(index(0), index(m_entries.size() - 1), {IsLikedRole});
}

void TrackListModel::refreshLikedState(int row, bool liked)
{
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    const QString songId = localSongIdAt(row);
    if (liked) {
        m_likedSongIds.insert(songId);
    } else {
        m_likedSongIds.remove(songId);
    }
    emit dataChanged(index(row), index(row), {IsLikedRole});
}

QString TrackListModel::filePathAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }
    return m_entries.at(row).filePath;
}

QString TrackListModel::localSongIdAt(int row) const
{
    return PlaylistStore::localSongId(filePathAt(row));
}
