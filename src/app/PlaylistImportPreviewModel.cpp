#include "app/PlaylistImportPreviewModel.h"

PlaylistImportPreviewModel::PlaylistImportPreviewModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int PlaylistImportPreviewModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_rows.size();
}

QVariant PlaylistImportPreviewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }

    const PlaylistImportPreviewRow& row = m_rows.at(index.row());
    switch (role) {
    case ExternalTitleRole:
        return row.externalTitle;
    case ExternalArtistRole:
        return row.externalArtist;
    case MatchedTitleRole:
        return row.match.title;
    case MatchedArtistRole:
        return row.match.artist;
    case SourceLabelRole:
        return row.sourceLabel;
    case StatusTextRole:
        if (row.imported) {
            return QStringLiteral("已导入");
        }
        if (row.duplicate) {
            return QStringLiteral("已存在");
        }
        return row.hasMatch() ? QStringLiteral("已匹配") : QStringLiteral("未匹配");
    case StatusKindRole:
        if (row.imported) {
            return QStringLiteral("imported");
        }
        if (row.duplicate) {
            return QStringLiteral("duplicate");
        }
        return row.hasMatch() ? QStringLiteral("matched") : QStringLiteral("missing");
    case HasMatchRole:
        return row.hasMatch();
    case ImportedRole:
        return row.imported;
    case DuplicateRole:
        return row.duplicate;
    default:
        return {};
    }
}

QHash<int, QByteArray> PlaylistImportPreviewModel::roleNames() const
{
    return {
        {ExternalTitleRole, "externalTitle"},
        {ExternalArtistRole, "externalArtist"},
        {MatchedTitleRole, "matchedTitle"},
        {MatchedArtistRole, "matchedArtist"},
        {SourceLabelRole, "sourceLabel"},
        {StatusTextRole, "statusText"},
        {StatusKindRole, "statusKind"},
        {HasMatchRole, "hasMatch"},
        {ImportedRole, "imported"},
        {DuplicateRole, "duplicate"},
    };
}

void PlaylistImportPreviewModel::clear()
{
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

void PlaylistImportPreviewModel::appendRow(const PlaylistImportPreviewRow& row)
{
    const int idx = m_rows.size();
    beginInsertRows({}, idx, idx);
    m_rows.append(row);
    endInsertRows();
}

void PlaylistImportPreviewModel::markImported(const QSet<QString>& importedIds,
                                              const QSet<QString>& duplicateIds)
{
    if (m_rows.isEmpty()) {
        return;
    }

    for (PlaylistImportPreviewRow& row : m_rows) {
        row.imported = importedIds.contains(row.match.songId);
        row.duplicate = duplicateIds.contains(row.match.songId);
    }
    emit dataChanged(index(0), index(m_rows.size() - 1),
                     {StatusTextRole, StatusKindRole, ImportedRole, DuplicateRole});
}

const QVector<PlaylistImportPreviewRow>& PlaylistImportPreviewModel::rows() const
{
    return m_rows;
}

int PlaylistImportPreviewModel::matchedCount() const
{
    int count = 0;
    for (const PlaylistImportPreviewRow& row : m_rows) {
        if (row.hasMatch()) {
            ++count;
        }
    }
    return count;
}

int PlaylistImportPreviewModel::unmatchedCount() const
{
    return m_rows.size() - matchedCount();
}
