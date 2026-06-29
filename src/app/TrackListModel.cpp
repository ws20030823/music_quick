#include "app/TrackListModel.h"

TrackListModel::TrackListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

// 返回列表行数（树形子节点返回 0）
int TrackListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

// 按角色返回单元格数据，供 QML delegate 绑定 title/artist 等
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
    case RowIndexRole:
        return index.row();
    default:
        return {};
    }
}

// 将 C++ 角色枚举映射为 QML 属性名
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
        {RowIndexRole, "rowIndex"},
    };
}

void TrackListModel::setTracks(QVector<TrackEntry> tracks)
{
    beginResetModel();
    m_entries = std::move(tracks);
    m_playingRow = -1;
    endResetModel();
}

const QVector<TrackEntry>& TrackListModel::entries() const
{
    return m_entries;
}

// 切换播放高亮：旧行取消、新行标记，局部刷新避免整表 reset
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
        const QModelIndex idx = index(r);
        emit dataChanged(idx, idx, {IsPlayingRole});
    };

    updateRow(m_playingRow, false);
    m_playingRow = row;
    updateRow(m_playingRow, true);
}

int TrackListModel::playingRow() const
{
    return m_playingRow;
}

QString TrackListModel::filePathAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }
    return m_entries.at(row).filePath;
}
