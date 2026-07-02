#include "app/SearchResultModel.h"

SearchResultModel::SearchResultModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int SearchResultModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_entries.size();
}

QVariant SearchResultModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }

    const SearchResultEntry& entry = m_entries.at(index.row());
    switch (role) {
    case FilePathRole:
        return entry.streamUrl;
    case TitleRole:
        return entry.metadata.title.isEmpty() ? entry.metadata.artist : entry.metadata.title;
    case ArtistRole:
        return entry.metadata.artist;
    case AlbumRole:
        return QStringLiteral("在线音乐");
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
        return entry.songId;
    case StreamUrlRole:
        return entry.streamUrl;
    case SourceLabelRole:
        return QStringLiteral("MyFreeMp3");
    default:
        return {};
    }
}

QHash<int, QByteArray> SearchResultModel::roleNames() const
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
        {SourceLabelRole, "sourceLabel"},
    };
}

void SearchResultModel::setResults(QVector<SearchResultEntry> entries)
{
    beginResetModel();
    m_entries = std::move(entries);
    m_playingRow = -1;
    endResetModel();
}

const QVector<SearchResultEntry>& SearchResultModel::entries() const
{
    return m_entries;
}

void SearchResultModel::setPlayingRow(int row)
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

int SearchResultModel::playingRow() const
{
    return m_playingRow;
}

QString SearchResultModel::songIdAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }
    return m_entries.at(row).songId;
}

QString SearchResultModel::streamUrlAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) {
        return {};
    }
    return m_entries.at(row).streamUrl;
}

void SearchResultModel::updateStreamUrl(int row, const QString& streamUrl, const QString& coverUrl)
{
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    m_entries[row].streamUrl = streamUrl;
    m_entries[row].coverUrl = coverUrl;
    emit dataChanged(index(row), index(row), {FilePathRole, StreamUrlRole});
}

void SearchResultModel::updateCover(int row, const QImage& cover)
{
    if (row < 0 || row >= m_entries.size() || cover.isNull()) {
        return;
    }
    m_entries[row].metadata.cover = cover;
    emit dataChanged(index(row), index(row), {HasCoverRole, CoverRole});
}
