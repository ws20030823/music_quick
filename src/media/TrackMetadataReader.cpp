#include "TrackMetadataReader.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QTimer>
#include <QUrl>

namespace {

// 毫秒 → "mm:ss"；无效时长统一显示 "--:--"
QString formatDurationMs(qint64 durationMs)
{
    if(durationMs <= 0)
    {
        return QStringLiteral("--:--");
    }

    const qint64 totalSeconds = durationMs / 1000;
    const qint64 minutes = totalSeconds / 60;
    const qint64 seconds = totalSeconds % 60;
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

// 不同格式/编码下歌手字段名不一致，按常见程度依次尝试
QString pickArtist(const QMediaMetaData& meta)
{
    const QList<QMediaMetaData::Key> keys{
        QMediaMetaData::ContributingArtist,
        QMediaMetaData::AlbumArtist,
        QMediaMetaData::Author,
    };

    for(const QMediaMetaData::Key key : keys)
    {
        const QString value = meta.stringValue(key);
        if(!value.isEmpty())
        {
            return value;
        }
    }
    return QString();
}

// 封面可能在 CoverArtImage 或 ThumbnailImage 中
QImage pickCover(const QMediaMetaData& meta)
{
    const QList<QMediaMetaData::Key> keys{
        QMediaMetaData::CoverArtImage,
        QMediaMetaData::ThumbnailImage,
    };

    for(const QMediaMetaData::Key key : keys)
    {
        const QVariant value = meta.value(key);
        if(value.canConvert<QImage>())
        {
            const QImage image = value.value<QImage>();
            if(!image.isNull())
            {
                return image;
            }
        }
    }
    return QImage();
}

// 将 QMediaMetaData / player.duration() 合并进 result，仅覆盖读到的非空字段
void applyMetaToResult(TrackMetadata& result, const QMediaMetaData& meta, qint64 durationMs)
{
    const QString title = meta.stringValue(QMediaMetaData::Title);
    if(!title.isEmpty())
    {
        result.title = title;
    }

    const QString artist = pickArtist(meta);
    if(!artist.isEmpty())
    {
        result.artist = artist;
    }

    const QString album = meta.stringValue(QMediaMetaData::AlbumTitle);
    if(!album.isEmpty())
    {
        result.album = album;
    }

    // duration() 通常比 meta 里的 Duration 更可靠，故优先用 player
    if(durationMs > 0)
    {
        result.durationText = formatDurationMs(durationMs);
    }
    else
    {
        const QVariant durationValue = meta.value(QMediaMetaData::Duration);
        if(durationValue.isValid())
        {
            result.durationText = formatDurationMs(durationValue.toLongLong());
        }
    }

    const QImage cover = pickCover(meta);
    if(!cover.isNull())
    {
        result.cover = cover;
    }
}

} // namespace

// 从本地文件同步读取元数据（内部用 QEventLoop 等待 QMediaPlayer 就绪）
TrackMetadata TrackMetadataReader::readFromFile(const QString& filePath, int timeoutMs)
{
    TrackMetadata result;
    const QFileInfo fileInfo(filePath);

    // 先填回退值，确保读失败时 UI 仍有可显示内容
    result.title = fileInfo.completeBaseName();
    result.artist = QStringLiteral("未知艺术家");
    result.album = QStringLiteral("未知专辑");
    result.durationText = QStringLiteral("--:--");

    if(!fileInfo.exists() || !fileInfo.isFile())
    {
        return result;
    }

    // QMediaPlayer 元数据是异步的：setSource 后通过信号通知就绪
    QMediaPlayer player;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    const auto finish = [&]() {
        if(loop.isRunning())
        {
            loop.quit();
        }
    };

    QObject::connect(&player, &QMediaPlayer::metaDataChanged, &loop, finish);
    QObject::connect(&player, &QMediaPlayer::durationChanged, &loop, finish);
    QObject::connect(&player, &QMediaPlayer::errorOccurred, &loop, finish);
    QObject::connect(&player, &QMediaPlayer::mediaStatusChanged, &loop, [&](QMediaPlayer::MediaStatus status) {
        if(status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::BufferedMedia)
        {
            finish();
        }
    });
    // 超时兜底：避免个别文件永远不触发信号而卡死导入流程
    QObject::connect(&timer, &QTimer::timeout, &loop, finish);

    player.setSource(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    timer.start(timeoutMs);
    loop.exec();

    applyMetaToResult(result, player.metaData(), player.duration());
    return result;
}
