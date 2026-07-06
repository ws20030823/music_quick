#pragma once

#include <QAbstractListModel>
#include <QSet>
#include <QVector>

#include "media/TrackMetadata.h"

// =============================================================================
// SearchResultModel — 在线搜索结果 QML 模型
// =============================================================================
// 角色名与 SongList.qml delegate 对齐，可直接复用 SongList 组件。
// filePath 字段在线场景下存储 streamUrl（解析后）或空字符串。
// =============================================================================

struct SearchResultEntry {
    QString songId;
    QString sourceId;
    QString sourceLabel;
    QString detailUrl;
    QString streamUrl;
    QString coverUrl;
    QString lyrics;
    TrackMetadata metadata;
    bool isPlaying = false;
    bool isSelected = false;
};

class SearchResultModel final : public QAbstractListModel
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
        SourceLabelRole,
        IsLikedRole,
        IsSelectedRole,
    };

    explicit SearchResultModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setResults(QVector<SearchResultEntry> entries);
    const QVector<SearchResultEntry>& entries() const;

    void setPlayingRow(int row);
    int playingRow() const;

    void setSelectedRow(int row);
    int selectedRow() const;

    QString songIdAt(int row) const;
    QString streamUrlAt(int row) const;
    void updateStreamUrl(int row,
                         const QString& streamUrl,
                         const QString& coverUrl,
                         const QString& lyrics = {});
    void updateCover(int row, const QImage& cover);
    void setLikedSongIds(const QSet<QString>& songIds);
    void refreshLikedState(int row, bool liked);

private:
    QVector<SearchResultEntry> m_entries;
    int m_playingRow = -1;
    int m_selectedRow = -1;
    QSet<QString> m_likedSongIds;
};
