// =============================================================================
// test_core.cpp — Music Quick 核心逻辑单元测试（无 QML / Widgets 依赖）
// =============================================================================
// 覆盖 AudioPlayer、PlaybackMode 纯函数、PlaybackController、TrackMetadataReader
// =============================================================================

#include <QtTest>
#include <QCoreApplication>
#include <QSignalSpy>

#include "core/AudioPlayer.h"
#include "core/PlaybackController.h"
#include "core/PlaybackMode.h"
#include "media/TrackMetadataReader.h"

// AudioPlayer：构造、音量、加载、toggle 等基础行为
class TestAudioPlayer final : public QObject
{
    Q_OBJECT

private slots:
    void initialStateIsStopped()
    {
        AudioPlayer player;
        QCOMPARE(player.playbackState(), QMediaPlayer::StoppedState);
    }

    void setVolumeAcceptsValidRange()
    {
        AudioPlayer player;
        player.setVolume(0);
        player.setVolume(50);
        player.setVolume(100);
        QVERIFY(true);
    }

    void loadMissingFileDoesNotCrash()
    {
        AudioPlayer player;
        player.load(QStringLiteral("Z:/__music_quick_missing_file__.mp3"));
        QVERIFY(player.playbackState() == QMediaPlayer::StoppedState
                || player.playbackState() == QMediaPlayer::PausedState);
    }

    void togglePlaybackWhenStoppedDoesNotCrash()
    {
        AudioPlayer player;
        player.togglePlayback();
        QVERIFY(true);
    }

    void playWithoutSourceDoesNotCrash()
    {
        AudioPlayer player;
        player.play();
        QVERIFY(true);
    }
};

// PlaybackMode.h 纯函数：下一首/循环/模式切换逻辑
class TestPlaybackNavigation final : public QObject
{
    Q_OBJECT

private slots:
    void repeatOffStopsAtListEnd()
    {
        const QVector<int> order{0, 1, 2};
        const auto result = resolveNextPlayback(2, 3, RepeatMode::Off, false, order, -1);
        QCOMPARE(result.row, -1);
        QVERIFY(!result.replayCurrent);
    }

    void repeatAllWrapsToFirstTrack()
    {
        const QVector<int> order{0, 1, 2};
        const auto result = resolveNextPlayback(2, 3, RepeatMode::All, false, order, -1);
        QCOMPARE(result.row, 0);
    }

    void repeatOneReplaysCurrentTrack()
    {
        const QVector<int> order{0, 1, 2};
        const auto result = resolveNextPlayback(1, 3, RepeatMode::One, false, order, -1);
        QCOMPARE(result.row, 1);
        QVERIFY(result.replayCurrent);
    }

    void shuffleUsesShuffleOrder()
    {
        const QVector<int> order{2, 0, 1};
        const auto result = resolveNextPlayback(2, 3, RepeatMode::Off, true, order, 0);
        QCOMPARE(result.row, 0);
        QCOMPARE(result.shuffleIndex, 1);
    }

    void cycleRepeatModeRotates()
    {
        QCOMPARE(cycleRepeatMode(RepeatMode::Off), RepeatMode::All);
        QCOMPARE(cycleRepeatMode(RepeatMode::All), RepeatMode::One);
        QCOMPARE(cycleRepeatMode(RepeatMode::One), RepeatMode::Off);
    }

    void playbackDisplayModeCycles()
    {
        QCOMPARE(cyclePlaybackDisplayMode(PlaybackDisplayMode::Sequential),
                 PlaybackDisplayMode::Shuffle);
        QCOMPARE(cyclePlaybackDisplayMode(PlaybackDisplayMode::Shuffle),
                 PlaybackDisplayMode::RepeatAll);
        QCOMPARE(cyclePlaybackDisplayMode(PlaybackDisplayMode::RepeatAll),
                 PlaybackDisplayMode::RepeatOne);
        QCOMPARE(cyclePlaybackDisplayMode(PlaybackDisplayMode::RepeatOne),
                 PlaybackDisplayMode::Sequential);
    }

    void applyPlaybackDisplayModeUpdatesState()
    {
        bool shuffle = true;
        RepeatMode repeat = RepeatMode::All;
        applyPlaybackDisplayMode(PlaybackDisplayMode::Sequential, shuffle, repeat);
        QCOMPARE(shuffle, false);
        QCOMPARE(repeat, RepeatMode::Off);
    }
};

// PlaybackController：有状态导航与 shuffle 重建
class TestPlaybackController final : public QObject
{
    Q_OBJECT

private slots:
    void controllerRepeatAllWrapsAtEnd()
    {
        PlaybackController controller;
        controller.setTrackCount(3);
        controller.state().repeatMode = RepeatMode::All;
        const PlaybackNavigateResult result = controller.navigateNext(2);
        QCOMPARE(result.row, 0);
        QVERIFY(!result.replayCurrent);
    }

    void controllerRepeatOneReplaysCurrent()
    {
        PlaybackController controller;
        controller.setTrackCount(3);
        controller.state().repeatMode = RepeatMode::One;
        const PlaybackNavigateResult result = controller.navigateNext(1);
        QCOMPARE(result.row, 1);
        QVERIFY(result.replayCurrent);
    }

    void toggleShuffleEmitsSignal()
    {
        PlaybackController controller;
        QSignalSpy spy(&controller, &PlaybackController::shuffleEnabledChanged);
        controller.toggleShuffle();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), true);
    }

    void cyclePlaybackModeRotatesThroughModes()
    {
        PlaybackController controller;
        QCOMPARE(playbackDisplayModeFromState(
                     controller.state().shuffleEnabled, controller.state().repeatMode),
                 PlaybackDisplayMode::Sequential);

        controller.cyclePlaybackMode();
        QCOMPARE(playbackDisplayModeFromState(
                     controller.state().shuffleEnabled, controller.state().repeatMode),
                 PlaybackDisplayMode::Shuffle);

        controller.cyclePlaybackMode();
        QCOMPARE(playbackDisplayModeFromState(
                     controller.state().shuffleEnabled, controller.state().repeatMode),
                 PlaybackDisplayMode::RepeatAll);

        controller.cyclePlaybackMode();
        QCOMPARE(playbackDisplayModeFromState(
                     controller.state().shuffleEnabled, controller.state().repeatMode),
                 PlaybackDisplayMode::RepeatOne);

        controller.cyclePlaybackMode();
        QCOMPARE(playbackDisplayModeFromState(
                     controller.state().shuffleEnabled, controller.state().repeatMode),
                 PlaybackDisplayMode::Sequential);
    }

    void shuffleRebuildAnchorsCurrentAndAllowsNext()
    {
        PlaybackController controller;
        controller.setTrackCount(3);
        controller.state().shuffleEnabled = true;
        controller.rebuildShuffleOrder(0);

        QCOMPARE(controller.state().shuffleOrder.first(), 0);
        QCOMPARE(controller.state().shuffleIndex, 0);

        const PlaybackNavigateResult result = controller.navigateNext(0);
        QVERIFY(result.row >= 0);
        QVERIFY(result.row != 0);
        QCOMPARE(result.row, controller.state().shuffleOrder.at(result.shuffleIndex));
    }
};

// TrackMetadataReader：异常文件回退元数据
class TestTrackMetadataReader final : public QObject
{
    Q_OBJECT

private slots:
    void missingFileReturnsFallbackMetadata()
    {
        const TrackMetadata metadata = TrackMetadataReader::readFromFile(
            QStringLiteral("Z:/__music_quick_missing_metadata__.mp3"), 100);
        QCOMPARE(metadata.artist, QStringLiteral("未知艺术家"));
        QCOMPARE(metadata.album, QStringLiteral("未知专辑"));
        QCOMPARE(metadata.durationText, QStringLiteral("--:--"));
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;

    {
        TestAudioPlayer audioPlayerTests;
        status |= QTest::qExec(&audioPlayerTests, argc, argv);
    }
    {
        TestPlaybackNavigation playbackNavigationTests;
        status |= QTest::qExec(&playbackNavigationTests, argc, argv);
    }
    {
        TestPlaybackController playbackControllerTests;
        status |= QTest::qExec(&playbackControllerTests, argc, argv);
    }
    {
        TestTrackMetadataReader trackMetadataReaderTests;
        status |= QTest::qExec(&trackMetadataReaderTests, argc, argv);
    }

    return status;
}

#include "test_core.moc"
