#include "AudioPlayer.h"
#include <QAudioOutput>
#include <QUrl>

// 封装 QMediaPlayer，对外只暴露播放控制和信号，UI 层不直接操作底层播放器
AudioPlayer::AudioPlayer(QObject* parent):QObject(parent),player(new QMediaPlayer(this)),audioOutput(new QAudioOutput(this))
{
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5);

    /*
    规则：
    1. 收发信号【参数个数、类型完全匹配】→ 直接信号绑定信号，简洁高效
    2. 收发信号【参数不匹配/需要过滤/转换数据】→ 必须Lambda做中间转接，手动emit自定义信号
    */
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &AudioPlayer::playbackStateChanged);
    connect(player, &QMediaPlayer::durationChanged,
            this, &AudioPlayer::durationChanged);
    connect(player, &QMediaPlayer::positionChanged,
            this, &AudioPlayer::positionChanged);
    connect(player,&QMediaPlayer::mediaStatusChanged,this,&AudioPlayer::mediaStatusChanged);
    /*
    原生QMediaPlayer::errorOccurred携带2个参数：(错误枚举码,错误文本)
    自定义AudioPlayer::errorOccurred只接收1个参数：(错误文本)
    参数数量对不齐，无法直接信号连信号；用Lambda手动截留、丢弃第一个参数，只转发字符串
    */
    connect(player, &QMediaPlayer::errorOccurred,
            this, [this](QMediaPlayer::Error, const QString& errorString) {
                emit errorOccurred(errorString);
    });
};

// 开始播放当前已加载的音源
void AudioPlayer::play()
{
    player->play();
};

// 切换音源前先 stop，避免上一首的播放状态残留
void AudioPlayer::load(const QString& sourcePath)
{
    loadUrl(QUrl::fromLocalFile(sourcePath));
}

void AudioPlayer::loadUrl(const QUrl& sourceUrl)
{
    player->stop();
    player->setSource(sourceUrl);
}

// 暂停当前播放
void AudioPlayer::pause()
{
    player->pause();
};

// 滑块范围 0~100，QAudioOutput 实际音量范围 0.0~1.0
void AudioPlayer::setVolume(int volume)
{
    audioOutput->setVolume(volume/100.0);
};

// 正在播放则暂停，否则开始播放（用于播放/暂停按钮）
void AudioPlayer::togglePlayback()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
        return;
    }
    player->play();
};

// 跳转到指定播放位置，position 单位毫秒
void AudioPlayer::setPosition(qint64 position)
{
    player->setPosition(position);
};

// 返回当前媒体总时长（毫秒）
qint64 AudioPlayer::duration() const
{
    return player->duration();
};

// 返回当前播放位置（毫秒）
qint64 AudioPlayer::position() const
{
    return player->position();
}

// 返回当前播放状态枚举
QMediaPlayer::PlaybackState AudioPlayer::playbackState() const
{
    return player->playbackState();
};