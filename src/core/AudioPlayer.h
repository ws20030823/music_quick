#pragma once

#include <QString>
#include <QMediaPlayer>
#include <QObject>
#include <QUrl>

// =============================================================================
// AudioPlayer — 音频播放封装（core 层）
// =============================================================================
// 封装 QMediaPlayer + QAudioOutput，对外提供 load/play/pause/seek/volume 等接口。
// 不依赖 UI；Widgets 版与 QML 版共用此实现。
// =============================================================================

class QAudioOutput;

class AudioPlayer final : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject* parent = nullptr);

    void load(const QString& sourcePath);
    // 加载 HTTP/HTTPS 串流或任意 QUrl 音源
    void loadUrl(const QUrl& sourceUrl);
    void play();
    void pause();
    void setVolume(int volume);
    void togglePlayback();
    void setPosition(qint64 position);

    qint64 duration() const;
    qint64 position() const;
    QMediaPlayer::PlaybackState playbackState() const;

signals:
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void errorOccurred(const QString& errorString);
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

private:
    QMediaPlayer* player;
    QAudioOutput* audioOutput;
};
