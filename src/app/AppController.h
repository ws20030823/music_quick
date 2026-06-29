#pragma once

// =============================================================================
// AppController — QML 与 core/media 的唯一桥接（app 层）
// =============================================================================
// 职责：
//   - 持有 AudioPlayer、PlaybackController、TrackListModel
//   - 通过 Q_PROPERTY / Q_INVOKABLE 暴露给 QML（contextProperty 名 "app"）
//   - 对应 Widgets 版 MainWindow 中的业务编排，但不依赖任何 QWidget
// =============================================================================

#include "app/TrackListModel.h"
#include "core/AudioPlayer.h"
#include "core/PlaybackController.h"
#include "core/PlaybackMode.h"

#include <QImage>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QMediaPlayer>

class AppController final : public QObject
{
    Q_OBJECT

    // --- QML 可绑定属性 ---
    Q_PROPERTY(TrackListModel* trackModel READ trackModel CONSTANT)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(bool canControl READ canControl NOTIFY canControlChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentSubtitle READ currentSubtitle NOTIFY nowPlayingChanged)
    Q_PROPERTY(bool hasCover READ hasCover NOTIFY nowPlayingChanged)
    Q_PROPERTY(QImage currentCover READ currentCover NOTIFY nowPlayingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(int playbackMode READ playbackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(QString playbackModeTooltip READ playbackModeTooltip NOTIFY playbackModeChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY trackCountChanged)
    Q_PROPERTY(bool queueVisible READ queueVisible WRITE setQueueVisible NOTIFY queueVisibleChanged)

public:
    explicit AppController(QObject* parent = nullptr);

    // 歌曲列表模型，供 ListView / SongList 绑定
    TrackListModel* trackModel();
    // 当前页面索引：0 首页 / 1 本地音乐 / 2 搜索
    int currentPage() const;
    void setCurrentPage(int page);

    bool isPlaying() const;
    // 是否已加载曲目，可启用播放控制按钮
    bool canControl() const;
    QString currentTitle() const;
    QString currentArtist() const;
    QString currentSubtitle() const;
    bool hasCover() const;
    QImage currentCover() const;
    qint64 position() const;
    qint64 duration() const;
    int volume() const;
    void setVolume(int value);
    // 播放模式枚举值，对应 PlaybackDisplayMode
    int playbackMode() const;
    QString playbackModeTooltip() const;
    QString statusText() const;
    int trackCount() const;
    bool queueVisible() const;
    void setQueueVisible(bool visible);

    // --- QML 可调用方法 ---
    Q_INVOKABLE void importFiles(const QList<QUrl>& urls);
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void playRow(int row);
    Q_INVOKABLE void playNext();
    Q_INVOKABLE void playPrevious();
    Q_INVOKABLE void cyclePlaybackMode();
    Q_INVOKABLE void seek(qint64 ms);
    // 返回队列面板用的 QVariantList（title/artist/duration/isPlaying/row）
    Q_INVOKABLE QVariantList queueItems() const;
    Q_INVOKABLE void playQueueRow(int row);

signals:
    void currentPageChanged();
    void playbackStateChanged();
    void nowPlayingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void playbackModeChanged();
    void statusTextChanged();
    void trackCountChanged();
    void queueVisibleChanged();
    void canControlChanged();

private:
    // 按行号加载音频并更新播放栏；autoPlay 控制是否立即播放
    void loadTrack(int row, bool autoPlay);
    // 同步曲目数并重建随机播放顺序
    void rebuildShuffleOrder();
    // 根据 navigateNext/Previous 结果切歌或重播/停止
    void applyNavigateResult(const PlaybackNavigateResult& result, int currentRow);
    // 从 PlaybackController 状态同步 playbackMode / tooltip
    void updatePlaybackModeProperties();
    void setStatus(const QString& text);

    TrackListModel m_trackModel;
    AudioPlayer m_audioPlayer;
    PlaybackController m_playbackController;

    int m_currentPage = 0;
    int m_currentRow = -1;
    QString m_currentFilePath;
    QString m_currentTitle;
    QString m_currentArtist;
    QString m_currentSubtitle;
    QImage m_currentCover;
    bool m_hasCover = false;
    bool m_canControl = false;
    int m_volume = 50;
    int m_playbackMode = static_cast<int>(PlaybackDisplayMode::Sequential);
    QString m_playbackModeTooltip = QStringLiteral("顺序播放");
    QString m_statusText = QStringLiteral("就绪");
    bool m_queueVisible = false;
};
