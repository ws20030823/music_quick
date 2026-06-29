#pragma once

// =============================================================================
// PlaybackQueueState.h — 播放队列运行时状态（struct，无行为）
// =============================================================================
// 由 PlaybackController 持有；UI 通过 controller->state() 只读访问（如队列面板、按钮外观）。
// =============================================================================

#include "PlaybackMode.h"
#include <QVector>

struct PlaybackQueueState {
    RepeatMode repeatMode = RepeatMode::Off;
    bool shuffleEnabled = false;
    QVector<int> shuffleOrder;   // 随机播放时的行号排列；关闭随机时为 0..n-1
    int shuffleIndex = -1;       // 当前曲在 shuffleOrder 中的下标
};
