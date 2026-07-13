#include "app/MediaCacheManager.h"

#include "app/AppStorage.h"

#include <algorithm>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QUrl>

namespace {

constexpr int kMinCacheMb = 64;
constexpr int kMaxCacheMb = 8192;

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

QString normalizedSongId(const QString& songId)
{
    return QCryptographicHash::hash(songId.toUtf8(), QCryptographicHash::Sha1).toHex();
}

struct CacheFileEntry {
    QString path;
    qint64 size = 0;
    qint64 modifiedMs = 0;
};

} // namespace

MediaCacheManager::MediaCacheManager(QObject* parent)
    : QObject(parent)
    , m_rootPath(defaultRootPath())
{
    loadSettings();
    ensureDirectories();
}

QString MediaCacheManager::defaultRootPath()
{
    const QString cacheLocation = AppStorage::cacheDir();
    if (!cacheLocation.isEmpty()) {
        return cacheLocation;
    }
    return QCoreApplication::applicationDirPath() + QStringLiteral("/cache");
}

QString MediaCacheManager::rootPath() const
{
    return m_rootPath;
}

void MediaCacheManager::setRootPath(const QString& path)
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty() || m_rootPath == trimmed) {
        return;
    }

    m_rootPath = trimmed;
    ensureDirectories();
    saveSettings();
    emit settingsChanged();
    emit usageChanged();
}

int MediaCacheManager::maxSizeMb() const
{
    return m_maxSizeMb;
}

void MediaCacheManager::setMaxSizeMb(int megabytes)
{
    const int clamped = qBound(kMinCacheMb, megabytes, kMaxCacheMb);
    if (m_maxSizeMb == clamped) {
        return;
    }

    m_maxSizeMb = clamped;
    saveSettings();
    emit settingsChanged();
    enforceLimit();
}

qint64 MediaCacheManager::usedBytes() const
{
    return directorySizeBytes(streamsDirectory()) + directorySizeBytes(coversDirectory());
}

QString MediaCacheManager::usedBytesText() const
{
    const qint64 bytes = usedBytes();
    if (bytes < 1024 * 1024) {
        return QStringLiteral("%1 KB").arg(bytes / 1024);
    }
    if (bytes < 1024LL * 1024 * 1024) {
        return QStringLiteral("%1 MB").arg(bytes / (1024 * 1024));
    }
    return QStringLiteral("%1 GB")
        .arg(static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}

QString MediaCacheManager::streamsDirectory() const
{
    return m_rootPath + QStringLiteral("/streams");
}

QString MediaCacheManager::coversDirectory() const
{
    return m_rootPath + QStringLiteral("/covers");
}

QString MediaCacheManager::streamFilePathFor(const QUrl& streamUrl) const
{
    const QByteArray hash = QCryptographicHash::hash(
        streamUrl.toString().toUtf8(), QCryptographicHash::Sha1).toHex();
    return streamsDirectory() + QLatin1Char('/') + QString::fromLatin1(hash) + extensionFromUrl(streamUrl);
}

QString MediaCacheManager::coverFilePathFor(const QString& songId) const
{
    if (songId.isEmpty()) {
        return {};
    }
    return coversDirectory() + QLatin1Char('/') + normalizedSongId(songId) + QStringLiteral(".jpg");
}

bool MediaCacheManager::streamCacheExists(const QUrl& streamUrl) const
{
    const QString path = streamFilePathFor(streamUrl);
    return QFileInfo::exists(path) && QFileInfo(path).size() > 1024;
}

bool MediaCacheManager::coverCacheExists(const QString& songId) const
{
    const QString path = coverFilePathFor(songId);
    return QFileInfo::exists(path) && QFileInfo(path).size() > 128;
}

void MediaCacheManager::ensureDirectories() const
{
    QDir().mkpath(streamsDirectory());
    QDir().mkpath(coversDirectory());
}

void MediaCacheManager::enforceLimit()
{
    const qint64 limitBytes = static_cast<qint64>(m_maxSizeMb) * 1024LL * 1024LL;
    const qint64 used = usedBytes();
    if (used <= limitBytes) {
        emit usageChanged();
        return;
    }

    removeOldestFiles(used - limitBytes);
    emit usageChanged();
}

void MediaCacheManager::clearAll()
{
    const auto removeDirFiles = [](const QString& dirPath) {
        QDir dir(dirPath);
        const QStringList files = dir.entryList(QDir::Files);
        for (const QString& name : files) {
            dir.remove(name);
        }
    };

    removeDirFiles(streamsDirectory());
    removeDirFiles(coversDirectory());
    emit usageChanged();
}

void MediaCacheManager::loadSettings()
{
    const auto settings = AppStorage::createSettings();
    const QString savedRoot = settings->value(QStringLiteral("cache/rootPath")).toString();
    if (!savedRoot.isEmpty()) {
        m_rootPath = savedRoot;
    }
    m_maxSizeMb = settings->value(QStringLiteral("cache/maxSizeMb"), 512).toInt();
    m_maxSizeMb = qBound(kMinCacheMb, m_maxSizeMb, kMaxCacheMb);
    ensureDirectories();
}

void MediaCacheManager::saveSettings() const
{
    const auto settings = AppStorage::createSettings();
    settings->setValue(QStringLiteral("cache/rootPath"), m_rootPath);
    settings->setValue(QStringLiteral("cache/maxSizeMb"), m_maxSizeMb);
}

qint64 MediaCacheManager::directorySizeBytes(const QString& path) const
{
    qint64 total = 0;
    const QDir dir(path);
    if (!dir.exists()) {
        return 0;
    }

    const QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& info : files) {
        total += info.size();
    }
    return total;
}

void MediaCacheManager::removeOldestFiles(qint64 bytesToFree)
{
    if (bytesToFree <= 0) {
        return;
    }

    QVector<CacheFileEntry> entries;
    const auto collect = [&](const QString& dirPath) {
        const QDir dir(dirPath);
        const QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        entries.reserve(entries.size() + files.size());
        for (const QFileInfo& info : files) {
            entries.append({info.absoluteFilePath(), info.size(), info.lastModified().toMSecsSinceEpoch()});
        }
    };

    collect(streamsDirectory());
    collect(coversDirectory());

    std::sort(entries.begin(), entries.end(), [](const CacheFileEntry& a, const CacheFileEntry& b) {
        return a.modifiedMs < b.modifiedMs;
    });

    qint64 freed = 0;
    for (const CacheFileEntry& entry : entries) {
        if (freed >= bytesToFree) {
            break;
        }
        if (QFile::remove(entry.path)) {
            freed += entry.size;
        }
    }
}
