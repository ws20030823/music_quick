#pragma once

// =============================================================================
// PlaybackController — 播放队列控制器（core 层）
// =============================================================================
// 持有 RepeatMode / shuffle 等运行时状态，并调用 PlaybackMode 纯函数计算目标行。
// AppController 把当前行交给 navigateNext/Previous，再根据结果切歌或更新 UI。
// 不依赖任何 QWidget / QML，便于单元测试。
// =============================================================================

#include "core/PlaybackMode.h"
#include "core/PlaybackQueueState.h"

#include <QObject>

class PlaybackController final : public QObject
{
    Q_OBJECT

public:
    explicit PlaybackController(QObject* parent = nullptr);

    PlaybackQueueState& state();
    const PlaybackQueueState& state() const;

    int trackCount() const;
    void setTrackCount(int count);

    void rebuildShuffleOrder(int currentRow);
    void syncShuffleIndexForRow(int row);
    void toggleShuffle();
    void cycleRepeat();
    // 四态循环：顺序 → 随机 → 列表循环 → 单曲循环
    void cyclePlaybackMode();

    PlaybackNavigateResult navigateNext(int currentRow);
    PlaybackNavigateResult navigatePrevious(int currentRow);

signals:
    void repeatModeChanged(RepeatMode mode);
    void shuffleEnabledChanged(bool enabled);

private:
    PlaybackQueueState m_state;
    int m_trackCount = 0;  // 与列表曲目数同步，由 AppController 写入
};
