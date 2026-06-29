#include "core/PlaybackController.h"

#include <QRandomGenerator>
#include <algorithm>

PlaybackController::PlaybackController(QObject* parent)
    : QObject(parent)
{
}

PlaybackQueueState& PlaybackController::state()
{
    return m_state;
}

const PlaybackQueueState& PlaybackController::state() const
{
    return m_state;
}

int PlaybackController::trackCount() const
{
    return m_trackCount;
}

// 由 AppController 在导入/列表变化时同步曲目总数
void PlaybackController::setTrackCount(int count)
{
    m_trackCount = count;
}

// 按列表行数重建 shuffleOrder；随机模式下当前曲固定队首，其余打乱
void PlaybackController::rebuildShuffleOrder(int currentRow)
{
    m_state.shuffleOrder.clear();

    if (m_trackCount <= 0) {
        m_state.shuffleIndex = -1;
        return;
    }

    if (!m_state.shuffleEnabled || m_trackCount <= 1) {
        for (int i = 0; i < m_trackCount; ++i) {
            m_state.shuffleOrder.append(i);
        }
        m_state.shuffleIndex = currentRow >= 0
            ? findShuffleIndex(m_state.shuffleOrder, currentRow)
            : -1;
        return;
    }

    QVector<int> rest;
    rest.reserve(m_trackCount - 1);
    for (int i = 0; i < m_trackCount; ++i) {
        if (i != currentRow) {
            rest.append(i);
        }
    }

    if (currentRow < 0) {
        m_state.shuffleOrder = rest;
        if (m_state.shuffleOrder.size() > 1) {
            std::shuffle(
                m_state.shuffleOrder.begin(),
                m_state.shuffleOrder.end(),
                *QRandomGenerator::global());
        }
        m_state.shuffleIndex = -1;
        return;
    }

    if (rest.size() > 1) {
        std::shuffle(rest.begin(), rest.end(), *QRandomGenerator::global());
    }

    m_state.shuffleOrder.append(currentRow);
    m_state.shuffleOrder += rest;
    m_state.shuffleIndex = 0;
}

// 用户点击列表某行时，让 shuffleIndex 与该行在随机序中的位置一致
void PlaybackController::syncShuffleIndexForRow(int row)
{
    if (!m_state.shuffleEnabled) {
        return;
    }
    m_state.shuffleIndex = findShuffleIndex(m_state.shuffleOrder, row);
}

void PlaybackController::toggleShuffle()
{
    m_state.shuffleEnabled = !m_state.shuffleEnabled;
    emit shuffleEnabledChanged(m_state.shuffleEnabled);
}

void PlaybackController::cycleRepeat()
{
    m_state.repeatMode = cycleRepeatMode(m_state.repeatMode);
    emit repeatModeChanged(m_state.repeatMode);
}

void PlaybackController::cyclePlaybackMode()
{
    const PlaybackDisplayMode current = playbackDisplayModeFromState(
        m_state.shuffleEnabled, m_state.repeatMode);
    const PlaybackDisplayMode next = cyclePlaybackDisplayMode(current);
    const bool oldShuffle = m_state.shuffleEnabled;
    const RepeatMode oldRepeat = m_state.repeatMode;
    applyPlaybackDisplayMode(next, m_state.shuffleEnabled, m_state.repeatMode);
    if (oldShuffle != m_state.shuffleEnabled) {
        emit shuffleEnabledChanged(m_state.shuffleEnabled);
    }
    if (oldRepeat != m_state.repeatMode) {
        emit repeatModeChanged(m_state.repeatMode);
    }
}

// 计算下一首目标行，并写回 shuffleIndex
PlaybackNavigateResult PlaybackController::navigateNext(int currentRow)
{
    const PlaybackNavigateResult result = resolveNextPlayback(
        currentRow,
        m_trackCount,
        m_state.repeatMode,
        m_state.shuffleEnabled,
        m_state.shuffleOrder,
        m_state.shuffleIndex);

    m_state.shuffleIndex = result.shuffleIndex;
    return result;
}

PlaybackNavigateResult PlaybackController::navigatePrevious(int currentRow)
{
    const PlaybackNavigateResult result = resolvePreviousPlayback(
        currentRow,
        m_trackCount,
        m_state.repeatMode,
        m_state.shuffleEnabled,
        m_state.shuffleOrder,
        m_state.shuffleIndex);

    m_state.shuffleIndex = result.shuffleIndex;
    return result;
}
