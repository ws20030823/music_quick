#pragma once

#include <QVector>

// =============================================================================
// PlaybackMode.h — 播放队列纯函数（无状态，可单测）
// =============================================================================
// RepeatMode 枚举与 resolveNext/PreviousPlayback：给定模式、当前行、随机序 → 目标行。
// 不持有 shuffle 状态；运行时状态由 PlaybackController 管理。
// =============================================================================

// 列表循环模式：关 / 列表循环 / 单曲循环
enum class RepeatMode {
    Off,
    All,
    One
};

// UI 播放模式按钮的四种展示态（顺序 → 随机 → 列表循环 → 单曲循环）
enum class PlaybackDisplayMode {
    Sequential,
    Shuffle,
    RepeatAll,
    RepeatOne,
};

// 播放模式按钮四态轮转：顺序 → 随机 → 列表循环 → 单曲循环
inline PlaybackDisplayMode cyclePlaybackDisplayMode(PlaybackDisplayMode mode)
{
    switch (mode) {
    case PlaybackDisplayMode::Sequential: return PlaybackDisplayMode::Shuffle;
    case PlaybackDisplayMode::Shuffle: return PlaybackDisplayMode::RepeatAll;
    case PlaybackDisplayMode::RepeatAll: return PlaybackDisplayMode::RepeatOne;
    case PlaybackDisplayMode::RepeatOne: return PlaybackDisplayMode::Sequential;
    }
    return PlaybackDisplayMode::Sequential;
}

// 从 shuffle/repeat 运行时状态推导 UI 展示模式
inline PlaybackDisplayMode playbackDisplayModeFromState(bool shuffleEnabled, RepeatMode repeat)
{
    if (shuffleEnabled) {
        return PlaybackDisplayMode::Shuffle;
    }
    if (repeat == RepeatMode::All) {
        return PlaybackDisplayMode::RepeatAll;
    }
    if (repeat == RepeatMode::One) {
        return PlaybackDisplayMode::RepeatOne;
    }
    return PlaybackDisplayMode::Sequential;
}

// 将 UI 展示模式写回 shuffleEnabled + repeatMode
inline void applyPlaybackDisplayMode(PlaybackDisplayMode mode, bool& shuffleEnabled, RepeatMode& repeat)
{
    switch (mode) {
    case PlaybackDisplayMode::Sequential:
        shuffleEnabled = false;
        repeat = RepeatMode::Off;
        break;
    case PlaybackDisplayMode::Shuffle:
        shuffleEnabled = true;
        repeat = RepeatMode::Off;
        break;
    case PlaybackDisplayMode::RepeatAll:
        shuffleEnabled = false;
        repeat = RepeatMode::All;
        break;
    case PlaybackDisplayMode::RepeatOne:
        shuffleEnabled = false;
        repeat = RepeatMode::One;
        break;
    }
}

inline RepeatMode cycleRepeatMode(RepeatMode mode)
{
    switch (mode) {
    case RepeatMode::Off: return RepeatMode::All;
    case RepeatMode::All: return RepeatMode::One;
    case RepeatMode::One: return RepeatMode::Off;
    }
    return RepeatMode::Off;
}

struct PlaybackNavigateResult {
    int row = -1;              // 目标列表行；-1 表示停止不切换
    int shuffleIndex = -1;     // 随机模式下的顺序下标
    bool replayCurrent = false; // 单曲循环：重播当前文件而非换行
};

// 在 shuffleOrder 中查找指定列表行对应的下标
inline int findShuffleIndex(const QVector<int>& shuffleOrder, int row)
{
    for (int i = 0; i < shuffleOrder.size(); ++i) {
        if (shuffleOrder[i] == row) {
            return i;
        }
    }
    return -1;
}

// 纯函数：给定当前行与模式，计算「下一首」目标行（无状态，可单测）
inline PlaybackNavigateResult resolveNextPlayback(
    int currentRow, int trackCount, RepeatMode repeat, bool shuffle,
    const QVector<int>& shuffleOrder, int shuffleIndex)
{
    PlaybackNavigateResult result;
    if (trackCount <= 0 || currentRow < 0 || currentRow >= trackCount) {
        return result;
    }

    if (repeat == RepeatMode::One) {
        result.replayCurrent = true;
        result.row = currentRow;
        result.shuffleIndex = shuffleIndex;
        return result;
    }

    if (shuffle) {
        if (shuffleOrder.isEmpty()) {
            return result;
        }
        int idx = shuffleIndex;
        if (idx < 0) {
            idx = findShuffleIndex(shuffleOrder, currentRow);
        }
        if (idx < 0) {
            idx = 0;
        }
        if (idx + 1 < shuffleOrder.size()) {
            result.shuffleIndex = idx + 1;
            result.row = shuffleOrder[result.shuffleIndex];
            return result;
        }
        if (repeat == RepeatMode::All) {
            result.shuffleIndex = 0;
            result.row = shuffleOrder[0];
        }
        return result;
    }

    const int nextRow = currentRow + 1;
    if (nextRow < trackCount) {
        result.row = nextRow;
        return result;
    }
    if (repeat == RepeatMode::All) {
        result.row = 0;
    }
    return result;
}

// 纯函数：给定当前行与模式，计算「上一首」目标行
inline PlaybackNavigateResult resolvePreviousPlayback(
    int currentRow, int trackCount, RepeatMode repeat, bool shuffle,
    const QVector<int>& shuffleOrder, int shuffleIndex)
{
    PlaybackNavigateResult result;
    if (trackCount <= 0 || currentRow < 0 || currentRow >= trackCount) {
        return result;
    }

    if (repeat == RepeatMode::One) {
        result.replayCurrent = true;
        result.row = currentRow;
        result.shuffleIndex = shuffleIndex;
        return result;
    }

    if (shuffle) {
        if (shuffleOrder.isEmpty()) {
            return result;
        }
        int idx = shuffleIndex;
        if (idx < 0) {
            idx = findShuffleIndex(shuffleOrder, currentRow);
        }
        if (idx < 0) {
            idx = 0;
        }
        if (idx > 0) {
            result.shuffleIndex = idx - 1;
            result.row = shuffleOrder[result.shuffleIndex];
            return result;
        }
        if (repeat == RepeatMode::All) {
            result.shuffleIndex = shuffleOrder.size() - 1;
            result.row = shuffleOrder[result.shuffleIndex];
        }
        return result;
    }

    const int previousRow = currentRow - 1;
    if (previousRow >= 0) {
        result.row = previousRow;
        return result;
    }
    if (repeat == RepeatMode::All) {
        result.row = trackCount - 1;
    }
    return result;
}
