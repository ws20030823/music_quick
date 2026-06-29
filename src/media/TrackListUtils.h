// TrackListUtils — Widgets 版列表工具（本 QML 项目未编入构建，保留供对照）
#pragma once

// =============================================================================
// TrackListUtils — 歌曲列表项与播放栏展示工具（media 层）
// =============================================================================
// 与 QListWidget / QListWidgetItem 相关的无状态纯函数。
// 不依赖 MainWindow，导入元数据、高亮当前行、封面与统计文案均可复用与单测。
// =============================================================================

#include "media/TrackMetadata.h"

#include <QImage>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QString;

namespace TrackListUtils {

// 将 TrackMetadata 写入 item 各 TrackItemRoles，供 Delegate 绘制
void applyMetadataToItem(QListWidgetItem* item,
                         const TrackMetadata& metadata,
                         const QString& filePath);
// 设置 IsPlaying 角色并刷新 viewport，驱动 Delegate 播放态高亮
void updateHighlight(QListWidget* list, int playingRow);
// 更新本地音乐页「共 N 首歌曲」文案
void updateLocalMusicStats(QLabel* statsLabel, const QListWidget* list);
// 播放栏封面：有图缩放显示，无图显示 ♪
void updateCoverLabel(QLabel* label, const QImage& cover);

} // namespace TrackListUtils
