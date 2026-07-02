#include "network/OnlineStreamLoader.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>

namespace {

constexpr auto kReferer = "https://myfreemp3online.com/";
constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicQuick/1.0";

QString extensionFromUrl(const QUrl& url)
{
    const QString path = url.path();
    const int dot = path.lastIndexOf(QLatin1Char('.'));
    if (dot >= 0) {
        const QString ext = path.mid(dot).toLower();
        if (ext == QStringLiteral(".mp3") || ext == QStringLiteral(".m4a")
            || ext == QStringLiteral(".aac") || ext == QStringLiteral(".wav")
            || ext == QStringLiteral(".flac")) {
            return ext;
        }
    }
    return QStringLiteral(".m4a");
}

} // namespace

OnlineStreamLoader::OnlineStreamLoader(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

QString OnlineStreamLoader::cacheFilePathFor(const QUrl& streamUrl) const
{
    const QByteArray hash = QCryptographicHash::hash(
        streamUrl.toString().toUtf8(), QCryptographicHash::Sha1).toHex();
    const QString base = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
        + QStringLiteral("/streams");
    QDir().mkpath(base);
    return base + QLatin1Char('/') + QString::fromLatin1(hash) + extensionFromUrl(streamUrl);
}

void OnlineStreamLoader::prefetch(const QUrl& streamUrl)
{
    if (!streamUrl.isValid()) {
        emit prefetchFailed(streamUrl, QStringLiteral("无效的播放地址"));
        return;
    }

    const QString cachePath = cacheFilePathFor(streamUrl);
    if (QFileInfo::exists(cachePath) && QFileInfo(cachePath).size() > 1024) {
        emit prefetchReady(cachePath, streamUrl);
        return;
    }

    QNetworkRequest request(streamUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
    request.setRawHeader("Referer", kReferer);
    request.setTransferTimeout(60000);

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, streamUrl, cachePath]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit prefetchFailed(streamUrl, reply->errorString());
            return;
        }

        const QByteArray data = reply->readAll();
        if (data.size() < 1024) {
            emit prefetchFailed(streamUrl, QStringLiteral("音频数据过短或无效"));
            return;
        }

        QFile file(cachePath);
        if (!file.open(QIODevice::WriteOnly)) {
            emit prefetchFailed(streamUrl, QStringLiteral("无法写入缓存文件"));
            return;
        }
        file.write(data);
        file.close();

        emit prefetchReady(cachePath, streamUrl);
    });
}
