#pragma once

// =============================================================================
// TrackListModel — 歌曲列表的 QML 数据模型（app 层）
// =============================================================================
// 将 QVector<TrackEntry> 暴露为 QAbstractListModel，供 QML ListView 绑定。
// 对应 Widgets 版的 QListWidget + TrackItemRoles，但使用 roleNames 供 QML 直接访问。
// =============================================================================

#include <QAbstractListModel>
#include <QVector>

#include "media/TrackMetadata.h"

// 单首歌曲在列表中的一行数据
struct TrackEntry {
    QString filePath;
    TrackMetadata metadata;
    bool isPlaying = false;
};

class TrackListModel final : public QAbstractListModel
{
    Q_OBJECT

public:
    // QML 可绑定的角色名（见 roleNames()）
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
    };

    explicit TrackListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 导入后一次性替换全部曲目
    void setTracks(QVector<TrackEntry> tracks);
    const QVector<TrackEntry>& entries() const;

    // 更新正在播放行的高亮（仅刷新 IsPlayingRole）
    void setPlayingRow(int row);
    int playingRow() const;

    QString filePathAt(int row) const;

private:
    QVector<TrackEntry> m_entries;
    int m_playingRow = -1;
};
