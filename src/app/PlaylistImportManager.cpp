#include "app/PlaylistImportManager.h"

#include "network/GequbaoClient.h"
#include "network/MusicSourceClient.h"
#include "network/MyFreeMp3Client.h"
#include "network/OnlineSongId.h"
#include "network/OnlineTrack.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace {

constexpr int kNeteaseDetailChunkSize = 120;

QString queryForTrack(const ExternalPlaylistTrack& track)
{
    return QStringLiteral("%1 %2").arg(track.title, track.artist).trimmed();
}

PlaylistTrackRef refFromOnlineTrack(const OnlineTrack& track)
{
    PlaylistTrackRef ref;
    ref.songId = track.songId;
    ref.sourceId = track.sourceId;
    if (ref.sourceId.isEmpty()) {
        QString sourceId;
        if (OnlineSongId::parse(track.songId, &sourceId, nullptr)) {
            ref.sourceId = sourceId;
        }
    }
    ref.title = track.title.isEmpty() ? track.displayTitle : track.title;
    ref.artist = track.artist;
    ref.album = QStringLiteral("在线音乐");
    ref.detailUrl = track.detailUrl;
    ref.streamUrl = track.streamUrl;
    ref.coverUrl = track.coverUrl;
    return ref;
}

} // namespace

PlaylistImportManager::PlaylistImportManager(QObject* parent)
    : QObject(parent)
{
    auto* gequbao = new GequbaoClient(this);
    auto* myFreeMp3 = new MyFreeMp3Client(this);
    m_sources = {gequbao, myFreeMp3};

    for (MusicSourceClient* source : m_sources) {
        connect(source, &MusicSourceClient::searchCompleted,
                this, [this, source](const SearchPageResult& result, const QString&) {
            Q_UNUSED(source)
            if (!m_busy || m_cancelled) {
                return;
            }
            if (!result.tracks.isEmpty()) {
                appendMatchedRow(result.tracks.first(), source->displayName());
                ++m_trackIndex;
                m_sourceIndex = 0;
                emit countsChanged();
                QTimer::singleShot(0, this, &PlaylistImportManager::matchCurrentTrack);
                return;
            }
            tryNextSource();
        });

        connect(source, &MusicSourceClient::searchFailed, this, [this, source](const QString&) {
            Q_UNUSED(source)
            if (!m_busy || m_cancelled) {
                return;
            }
            tryNextSource();
        });
    }
}

PlaylistImportPreviewModel* PlaylistImportManager::model()
{
    return &m_model;
}

bool PlaylistImportManager::busy() const { return m_busy; }
QString PlaylistImportManager::status() const { return m_status; }
QString PlaylistImportManager::platformName() const { return m_platformName; }
int PlaylistImportManager::matchedCount() const { return m_model.matchedCount(); }
int PlaylistImportManager::unmatchedCount() const { return m_model.unmatchedCount(); }
int PlaylistImportManager::processedCount() const { return m_model.rowCount(); }
int PlaylistImportManager::totalCount() const { return m_tracks.size(); }

void PlaylistImportManager::preview(const QString& rawUrl)
{
    if (m_busy) {
        cancel();
    }

    resetRunState();
    m_model.clear();

    const ExternalPlaylistRef ref = parseExternalPlaylistUrl(rawUrl);
    if (!ref.isValid()) {
        setStatus(QStringLiteral("无法识别的歌单链接"));
        emit previewFailed(m_status);
        return;
    }

    setBusy(true);
    setPlatformName(ref.platformName());
    setStatus(QStringLiteral("正在读取%1歌单…").arg(ref.platformName()));
    fetchExternalPlaylist(ref);
}

void PlaylistImportManager::cancel()
{
    m_cancelled = true;
    if (m_activeReply) {
        m_activeReply->abort();
    }
    for (MusicSourceClient* source : m_sources) {
        source->cancelResolveStreamUrl();
    }
    setBusy(false);
    setStatus(QStringLiteral("已取消导入预览"));
}

void PlaylistImportManager::clear()
{
    cancel();
    resetRunState();
    m_model.clear();
    setPlatformName({});
    setStatus(QStringLiteral("贴上网易云或 QQ 音乐歌单链接开始导入"));
    emit countsChanged();
}

void PlaylistImportManager::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    emit busyChanged();
}

void PlaylistImportManager::setStatus(const QString& status)
{
    if (m_status == status) {
        return;
    }
    m_status = status;
    emit statusChanged();
}

void PlaylistImportManager::setPlatformName(const QString& name)
{
    if (m_platformName == name) {
        return;
    }
    m_platformName = name;
    emit platformChanged();
}

void PlaylistImportManager::resetRunState()
{
    m_cancelled = false;
    m_tracks.clear();
    m_pendingNeteaseIds.clear();
    m_collectedNeteaseDetails.clear();
    m_detailOffset = 0;
    m_trackIndex = 0;
    m_sourceIndex = 0;
}

void PlaylistImportManager::fetchExternalPlaylist(const ExternalPlaylistRef& ref)
{
    const QUrl url = ref.platform == ExternalPlaylistPlatform::Netease
        ? buildNeteasePlaylistUrl(ref.id)
        : buildQQPlaylistUrl(ref.id);
    const ExternalPlaylistHeaders headers = ref.platform == ExternalPlaylistPlatform::Netease
        ? neteasePlaylistHeaders()
        : qqPlaylistHeaders();

    QNetworkReply* reply = m_network.get(buildRequest(url, headers));
    m_activeReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, ref]() {
        handleExternalReply(reply, ref, false);
    });
}

void PlaylistImportManager::fetchNeteaseSongDetailChunk()
{
    if (m_cancelled) {
        return;
    }

    if (m_detailOffset >= m_pendingNeteaseIds.size()) {
        m_tracks = m_collectedNeteaseDetails;
        startMatching();
        return;
    }

    const QVector<qint64> chunk = m_pendingNeteaseIds.mid(m_detailOffset, kNeteaseDetailChunkSize);
    m_detailOffset += chunk.size();
    setStatus(QStringLiteral("正在补全网易云曲目信息 %1/%2…")
                  .arg(qMin(m_detailOffset, m_pendingNeteaseIds.size()))
                  .arg(m_pendingNeteaseIds.size()));

    ExternalPlaylistRef ref;
    ref.platform = ExternalPlaylistPlatform::Netease;
    QNetworkReply* reply = m_network.get(buildRequest(buildNeteaseSongDetailUrl(chunk),
                                                      neteasePlaylistHeaders()));
    m_activeReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, ref]() {
        handleExternalReply(reply, ref, true);
    });
}

void PlaylistImportManager::handleExternalReply(QNetworkReply* reply,
                                                const ExternalPlaylistRef& ref,
                                                bool songDetailReply)
{
    reply->deleteLater();
    if (reply != m_activeReply) {
        return;
    }
    m_activeReply = nullptr;

    if (m_cancelled) {
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        setBusy(false);
        setStatus(reply->errorString());
        emit previewFailed(m_status);
        return;
    }

    const QByteArray body = reply->readAll();
    ExternalPlaylistParseResult result;
    if (songDetailReply) {
        result = parseNeteaseSongDetailJson(body);
    } else if (ref.platform == ExternalPlaylistPlatform::Netease) {
        result = parseNeteasePlaylistJson(body);
    } else {
        result = parseQQPlaylistJson(body);
    }

    if (!result.error.isEmpty()) {
        setBusy(false);
        setStatus(result.error);
        emit previewFailed(result.error);
        return;
    }

    if (songDetailReply) {
        m_collectedNeteaseDetails += result.tracks;
        fetchNeteaseSongDetailChunk();
        return;
    }

    if (!result.trackIds.isEmpty()) {
        m_pendingNeteaseIds = result.trackIds;
        fetchNeteaseSongDetailChunk();
        return;
    }

    m_tracks = result.tracks;
    startMatching();
}

QNetworkRequest PlaylistImportManager::buildRequest(const QUrl& url,
                                                    const ExternalPlaylistHeaders& headers) const
{
    QNetworkRequest request(url);
    for (const auto& item : headers) {
        request.setRawHeader(item.first, item.second);
    }
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(30000);
    return request;
}

void PlaylistImportManager::startMatching()
{
    if (m_tracks.isEmpty()) {
        setBusy(false);
        setStatus(QStringLiteral("歌单内没有可读取的曲目"));
        emit previewFailed(m_status);
        return;
    }

    m_trackIndex = 0;
    m_sourceIndex = 0;
    emit countsChanged();
    matchCurrentTrack();
}

void PlaylistImportManager::matchCurrentTrack()
{
    if (m_cancelled) {
        return;
    }
    if (m_trackIndex >= m_tracks.size()) {
        setBusy(false);
        setStatus(QStringLiteral("预览完成：匹配 %1 首，未匹配 %2 首")
                      .arg(m_model.matchedCount())
                      .arg(m_model.unmatchedCount()));
        emit countsChanged();
        emit previewCompleted();
        return;
    }

    m_sourceIndex = 0;
    setStatus(QStringLiteral("正在匹配 %1/%2：%3")
                  .arg(m_trackIndex + 1)
                  .arg(m_tracks.size())
                  .arg(m_tracks.at(m_trackIndex).title));
    tryNextSource();
}

void PlaylistImportManager::tryNextSource()
{
    if (m_cancelled) {
        return;
    }
    if (m_sourceIndex >= m_sources.size()) {
        appendMissingRow();
        ++m_trackIndex;
        m_sourceIndex = 0;
        emit countsChanged();
        QTimer::singleShot(0, this, &PlaylistImportManager::matchCurrentTrack);
        return;
    }

    MusicSourceClient* source = m_sources.at(m_sourceIndex);
    ++m_sourceIndex;
    source->search(queryForTrack(m_tracks.at(m_trackIndex)), 1);
}

void PlaylistImportManager::appendMissingRow()
{
    const ExternalPlaylistTrack& external = m_tracks.at(m_trackIndex);
    PlaylistImportPreviewRow row;
    row.externalTitle = external.title;
    row.externalArtist = external.artist;
    m_model.appendRow(row);
}

void PlaylistImportManager::appendMatchedRow(const OnlineTrack& track, const QString& sourceLabel)
{
    const ExternalPlaylistTrack& external = m_tracks.at(m_trackIndex);
    PlaylistImportPreviewRow row;
    row.externalTitle = external.title;
    row.externalArtist = external.artist;
    row.match = refFromOnlineTrack(track);
    row.sourceLabel = sourceLabel;
    m_model.appendRow(row);
}
