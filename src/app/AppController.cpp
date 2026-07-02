#include "app/AppController.h"

#include "media/TrackMetadataReader.h"
#include "network/MyFreeMp3Client.h"
#include "network/OnlineStreamLoader.h"

#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {

// 根据展示模式返回播放模式按钮的 tooltip 文案
QString tooltipForMode(PlaybackDisplayMode mode)
{
    switch (mode) {
    case PlaybackDisplayMode::Sequential:
        return QStringLiteral("顺序播放");
    case PlaybackDisplayMode::Shuffle:
        return QStringLiteral("随机播放");
    case PlaybackDisplayMode::RepeatAll:
        return QStringLiteral("列表循环");
    case PlaybackDisplayMode::RepeatOne:
        return QStringLiteral("单曲循环");
    }
    return QStringLiteral("顺序播放");
}

} // namespace

// 构造函数：初始化音量、播放模式，并连接 AudioPlayer 信号到 QML 属性通知
AppController::AppController(QObject* parent)
    : QObject(parent)
    , m_playbackController(this)
    , m_myFreeMp3Client(new MyFreeMp3Client(this))
    , m_streamLoader(new OnlineStreamLoader(this))
{
    m_audioPlayer.setVolume(m_volume);
    updatePlaybackModeProperties();

    connect(m_myFreeMp3Client, &MyFreeMp3Client::searchCompleted,
            this, &AppController::onSearchCompleted);
    connect(m_myFreeMp3Client, &MyFreeMp3Client::searchFailed,
            this, &AppController::onSearchFailed);
    connect(m_myFreeMp3Client, &MyFreeMp3Client::streamUrlResolved,
            this, &AppController::onStreamUrlResolved);
    connect(m_myFreeMp3Client, &MyFreeMp3Client::streamUrlFailed,
            this, &AppController::onStreamUrlFailed);
    connect(m_streamLoader, &OnlineStreamLoader::prefetchReady,
            this, &AppController::onStreamPrefetchReady);
    connect(m_streamLoader, &OnlineStreamLoader::prefetchFailed,
            this, &AppController::onStreamPrefetchFailed);

    connect(&m_audioPlayer, &AudioPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        emit playbackStateChanged();
        if (state == QMediaPlayer::PlayingState) {
            m_currentSubtitle = QStringLiteral("播放中");
        } else if (state == QMediaPlayer::PausedState) {
            m_currentSubtitle = QStringLiteral("已暂停");
        } else if (m_currentSubtitle != QStringLiteral("播放完成")) {
            m_currentSubtitle = QStringLiteral("已停止");
        }
        emit nowPlayingChanged();
    });

    connect(&m_audioPlayer, &AudioPlayer::errorOccurred, this, [this](const QString& error) {
        setStatus(QStringLiteral("播放失败：%1").arg(error));
    });

    connect(&m_audioPlayer, &AudioPlayer::durationChanged, this, &AppController::durationChanged);
    connect(&m_audioPlayer, &AudioPlayer::positionChanged, this, &AppController::positionChanged);

    // 一首播完：委托 PlaybackController 计算下一首
    connect(&m_audioPlayer, &AudioPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::EndOfMedia || m_currentRow < 0) {
            return;
        }
        const PlaybackNavigateResult result = m_playbackController.navigateNext(m_currentRow);
        applyNavigateResult(result, m_currentRow);
    });
}

AppController::~AppController() = default;

TrackListModel* AppController::trackModel()
{
    return &m_trackModel;
}

SearchResultModel* AppController::searchResultModel()
{
    return &m_searchResultModel;
}

int AppController::currentPage() const { return m_currentPage; }

void AppController::setCurrentPage(int page)
{
    if (m_currentPage == page) {
        return;
    }
    m_currentPage = page;
    emit currentPageChanged();
}

bool AppController::isPlaying() const
{
    return m_audioPlayer.playbackState() == QMediaPlayer::PlayingState;
}

bool AppController::canControl() const { return m_canControl; }

QString AppController::currentTitle() const { return m_currentTitle; }

QString AppController::currentArtist() const { return m_currentArtist; }

QString AppController::currentSubtitle() const { return m_currentSubtitle; }

bool AppController::hasCover() const { return m_hasCover; }

QImage AppController::currentCover() const { return m_currentCover; }

qint64 AppController::position() const { return m_audioPlayer.position(); }

qint64 AppController::duration() const { return m_audioPlayer.duration(); }

int AppController::volume() const { return m_volume; }

// 设置音量（0~100），并转发给 AudioPlayer
void AppController::setVolume(int value)
{
    const int clamped = qBound(0, value, 100);
    if (m_volume == clamped) {
        return;
    }
    m_volume = clamped;
    m_audioPlayer.setVolume(m_volume);
    emit volumeChanged();
}

int AppController::playbackMode() const { return m_playbackMode; }

QString AppController::playbackModeTooltip() const { return m_playbackModeTooltip; }

QString AppController::statusText() const { return m_statusText; }

int AppController::trackCount() const { return m_trackModel.rowCount(); }

bool AppController::queueVisible() const { return m_queueVisible; }

void AppController::setQueueVisible(bool visible)
{
    if (m_queueVisible == visible) {
        return;
    }
    m_queueVisible = visible;
    emit queueVisibleChanged();
}

bool AppController::searchBusy() const { return m_searchBusy; }
QString AppController::searchKeyword() const { return m_searchKeyword; }
QString AppController::searchStatus() const { return m_searchStatus; }
int AppController::searchCurrentPage() const { return m_searchCurrentPage; }
bool AppController::searchHasNext() const { return m_searchHasNext; }
bool AppController::searchHasPrevious() const { return m_searchHasPrevious; }
int AppController::searchResultCount() const { return m_searchResultModel.rowCount(); }

void AppController::setSearchBusy(bool busy)
{
    if (m_searchBusy == busy) {
        return;
    }
    m_searchBusy = busy;
    emit searchBusyChanged();
}

void AppController::setSearchStatus(const QString& text)
{
    if (m_searchStatus == text) {
        return;
    }
    m_searchStatus = text;
    emit searchStatusChanged();
}

// 向 MyFreeMp3 search.php 发起搜索；page 从 1 开始
void AppController::searchOnline(const QString& keyword, int page)
{
    const QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty()) {
        setSearchStatus(QStringLiteral("请输入歌名或歌手"));
        return;
    }

    m_searchKeyword = trimmed;
    emit searchKeywordChanged();

    setSearchBusy(true);
    setSearchStatus(QStringLiteral("搜索中…"));
    m_myFreeMp3Client->search(trimmed, page);
}

void AppController::searchNextPage()
{
    if (!m_searchHasNext || m_searchKeyword.isEmpty()) {
        return;
    }
    searchOnline(m_searchKeyword, m_searchCurrentPage + 1);
}

void AppController::searchPreviousPage()
{
    if (!m_searchHasPrevious || m_searchKeyword.isEmpty()) {
        return;
    }
    searchOnline(m_searchKeyword, m_searchCurrentPage - 1);
}

void AppController::applySearchPageResult(const SearchPageResult& result)
{
    QVector<SearchResultEntry> entries;
    entries.reserve(result.tracks.size());
    for (const OnlineTrack& track : result.tracks) {
        SearchResultEntry entry;
        entry.songId = track.songId;
        entry.streamUrl = track.streamUrl;
        entry.coverUrl = track.coverUrl;
        entry.metadata.title = track.title.isEmpty() ? track.displayTitle : track.title;
        entry.metadata.artist = track.artist;
        entry.metadata.album = QStringLiteral("在线音乐");
        entry.metadata.durationText = QStringLiteral("--:--");
        entries.append(entry);
    }

    m_searchResultModel.setResults(std::move(entries));
    m_searchCurrentPage = result.currentPage;
    m_searchHasNext = result.hasNext;
    m_searchHasPrevious = result.hasPrevious;
    emit searchPaginationChanged();
    emit searchResultCountChanged();
}

void AppController::onSearchCompleted(const SearchPageResult& result, const QString& keyword)
{
    Q_UNUSED(keyword);
    applySearchPageResult(result);
    setSearchBusy(false);

    if (result.tracks.isEmpty()) {
        setSearchStatus(QStringLiteral("未找到「%1」的相关歌曲，试试其他关键词").arg(m_searchKeyword));
        return;
    }
    setSearchStatus(QStringLiteral("找到 %1 首 · 第 %2 页")
                        .arg(result.tracks.size())
                        .arg(result.currentPage));
}

void AppController::onSearchFailed(const QString& message)
{
    setSearchBusy(false);
    setSearchStatus(QStringLiteral("搜索失败：%1").arg(message));
}

// 点击搜索结果：有缓存 URL 直接播，否则请求详情页解析
void AppController::playSearchRow(int row)
{
    if (row < 0 || row >= m_searchResultModel.rowCount()) {
        return;
    }

    m_isOnlinePlayback = true;
    m_pendingSearchRow = row;
    m_searchResultModel.setPlayingRow(row);

    const QVector<SearchResultEntry>& entries = m_searchResultModel.entries();
    const SearchResultEntry& entry = entries.at(row);
    m_currentTitle = entry.metadata.title;
    m_currentArtist = entry.metadata.artist.isEmpty()
        ? QStringLiteral("在线音乐")
        : entry.metadata.artist;

    const QString cachedUrl = entry.streamUrl;
    if (!cachedUrl.isEmpty()) {
        loadOnlineStream(cachedUrl, m_currentTitle, m_currentArtist, entry.metadata.cover, true);
        return;
    }

    setSearchBusy(true);
    setSearchStatus(QStringLiteral("正在获取播放地址…"));
    m_myFreeMp3Client->resolveStreamUrl(entry.songId);
}

void AppController::onStreamUrlResolved(const OnlineTrack& track)
{
    setSearchBusy(false);

    if (m_pendingSearchRow < 0) {
        return;
    }

    m_searchResultModel.updateStreamUrl(m_pendingSearchRow, track.streamUrl, track.coverUrl);
    if (!track.coverUrl.isEmpty()) {
        downloadSearchCover(m_pendingSearchRow, QUrl(track.coverUrl));
    }

    QImage cover;
    if (m_pendingSearchRow < m_searchResultModel.entries().size()) {
        cover = m_searchResultModel.entries().at(m_pendingSearchRow).metadata.cover;
    }

    const QString title = track.title.isEmpty() ? track.displayTitle : track.title;
    const QString artist = track.artist.isEmpty() ? QStringLiteral("在线音乐") : track.artist;
    setSearchStatus(QStringLiteral("正在播放…"));
    loadOnlineStream(track.streamUrl, title, artist, cover, true);
}

void AppController::onStreamUrlFailed(const QString& songId, const QString& message)
{
    Q_UNUSED(songId);
    setSearchBusy(false);
    setSearchStatus(QStringLiteral("无法播放：%1").arg(message));
}

void AppController::downloadSearchCover(int row, const QUrl& coverUrl)
{
    if (!coverUrl.isValid()) {
        return;
    }

    QNetworkRequest request(coverUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 MusicQuick/1.0"));
    request.setTransferTimeout(10000);

    QNetworkReply* reply = m_coverNetwork.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, row]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }
        QImage image;
        if (image.loadFromData(reply->readAll())) {
            m_searchResultModel.updateCover(row, image);
            if (m_isOnlinePlayback && m_pendingSearchRow == row) {
                m_currentCover = image;
                m_hasCover = true;
                emit nowPlayingChanged();
            }
        }
    });
}

void AppController::loadOnlineStream(const QString& streamUrl,
                                     const QString& title,
                                     const QString& artist,
                                     const QImage& cover,
                                     bool autoPlay)
{
    if (streamUrl.isEmpty() || !m_streamLoader) {
        return;
    }

    // CDN 拒绝无 Referer 的直接串流（403），先预取到本地缓存再交给 QMediaPlayer
    m_isOnlinePlayback = true;
    m_currentRow = -1;
    m_currentFilePath = streamUrl;
    m_currentTitle = title;
    m_currentArtist = artist;
    m_currentCover = cover;
    m_hasCover = !cover.isNull();
    m_pendingStreamAutoPlay = autoPlay;

    if (!m_canControl) {
        m_canControl = true;
        emit canControlChanged();
    }

    m_currentSubtitle = QStringLiteral("缓冲中…");
    emit nowPlayingChanged();
    setStatus(QStringLiteral("正在缓冲在线音频…"));
    setSearchStatus(QStringLiteral("正在缓冲…"));

    m_streamLoader->prefetch(QUrl(streamUrl));
}

void AppController::onStreamPrefetchReady(const QString& localFilePath, const QUrl& originalUrl)
{
    Q_UNUSED(originalUrl);

    m_currentFilePath = localFilePath;
    m_audioPlayer.load(localFilePath);

    m_currentSubtitle = m_pendingStreamAutoPlay
        ? QStringLiteral("在线播放中")
        : QStringLiteral("已加载");
    emit nowPlayingChanged();
    setStatus(QStringLiteral("在线播放"));
    setSearchStatus(QStringLiteral("正在播放…"));

    if (m_pendingStreamAutoPlay) {
        m_audioPlayer.play();
    }
}

void AppController::onStreamPrefetchFailed(const QUrl& originalUrl, const QString& message)
{
    Q_UNUSED(originalUrl);
    setSearchStatus(QStringLiteral("缓冲失败：%1").arg(message));
    setStatus(QStringLiteral("播放失败：%1").arg(message));
    m_currentSubtitle = QStringLiteral("播放失败");
    emit nowPlayingChanged();
}

// 多选导入：逐首读取元数据，默认加载第一首但不自动播放
void AppController::importFiles(const QList<QUrl>& urls)
{
    if (urls.isEmpty()) {
        return;
    }

    setStatus(QStringLiteral("正在读取元数据…"));

    QVector<TrackEntry> tracks;
    tracks.reserve(urls.size());
    for (const QUrl& url : urls) {
        const QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        if (path.isEmpty()) {
            continue;
        }
        TrackEntry entry;
        entry.filePath = path;
        entry.metadata = TrackMetadataReader::readFromFile(path);
        tracks.append(entry);
    }

    if (tracks.isEmpty()) {
        setStatus(QStringLiteral("未选择有效文件"));
        return;
    }

    m_trackModel.setTracks(std::move(tracks));
    emit trackCountChanged();

    rebuildShuffleOrder();
    m_trackModel.setPlayingRow(0);
    loadTrack(0, false);
    setStatus(QStringLiteral("已导入 %1 首歌曲").arg(m_trackModel.rowCount()));
}

void AppController::togglePlayback()
{
    m_audioPlayer.togglePlayback();
}

// 播放指定列表行：同步随机索引、更新高亮、加载并自动播放
void AppController::playRow(int row)
{
    if (row < 0 || row >= m_trackModel.rowCount()) {
        return;
    }

    const QString path = m_trackModel.filePathAt(row);
    if (path.isEmpty()) {
        return;
    }

    m_isOnlinePlayback = false;
    m_pendingSearchRow = -1;
    m_searchResultModel.setPlayingRow(-1);

    m_playbackController.syncShuffleIndexForRow(row);
    m_trackModel.setPlayingRow(row);
    loadTrack(row, true);
}

void AppController::playNext()
{
    if (m_currentRow < 0) {
        return;
    }

    const PlaybackNavigateResult result = m_playbackController.navigateNext(m_currentRow);
    applyNavigateResult(result, m_currentRow);
}

void AppController::playPrevious()
{
    if (m_currentRow < 0) {
        return;
    }

    const PlaybackNavigateResult result = m_playbackController.navigatePrevious(m_currentRow);
    applyNavigateResult(result, m_currentRow);
}

// 顺序 → 随机 → 列表循环 → 单曲循环，并重建随机序
void AppController::cyclePlaybackMode()
{
    m_playbackController.cyclePlaybackMode();
    rebuildShuffleOrder();
    updatePlaybackModeProperties();
    emit playbackModeChanged();
}

void AppController::seek(qint64 ms)
{
    m_audioPlayer.setPosition(ms);
}

// 构建播放队列弹窗所需的数据列表
QVariantList AppController::queueItems() const
{
    QVariantList items;
    const QVector<TrackEntry>& entries = m_trackModel.entries();
    const int playingRow = m_trackModel.playingRow();

    for (int i = 0; i < entries.size(); ++i) {
        const TrackEntry& entry = entries.at(i);
        QVariantMap map;
        map.insert(QStringLiteral("title"), entry.metadata.title);
        map.insert(QStringLiteral("artist"), entry.metadata.artist);
        map.insert(QStringLiteral("duration"), entry.metadata.durationText);
        map.insert(QStringLiteral("isPlaying"), i == playingRow);
        map.insert(QStringLiteral("row"), i);
        items.append(map);
    }
    return items;
}

// 从队列面板选中某行播放，并关闭弹窗
void AppController::playQueueRow(int row)
{
    playRow(row);
    setQueueVisible(false);
}

// 加载指定行的音频文件，更新底部播放栏元数据
void AppController::loadTrack(int row, bool autoPlay)
{
    const QString path = m_trackModel.filePathAt(row);
    if (path.isEmpty()) {
        return;
    }

    m_isOnlinePlayback = false;
    m_currentRow = row;
    m_currentFilePath = path;
    m_audioPlayer.load(path);

    const QVector<TrackEntry>& entries = m_trackModel.entries();
    if (row >= 0 && row < entries.size()) {
        const TrackEntry& entry = entries.at(row);
        m_currentTitle = entry.metadata.title;
        m_currentArtist = entry.metadata.artist;
        m_currentCover = entry.metadata.cover;
        m_hasCover = !entry.metadata.cover.isNull();
    } else {
        const QFileInfo info(path);
        m_currentTitle = info.fileName();
        m_currentArtist = QStringLiteral("未知艺术家");
        m_currentCover = {};
        m_hasCover = false;
    }

    if (!m_canControl) {
        m_canControl = true;
        emit canControlChanged();
    }

    m_currentSubtitle = autoPlay ? QStringLiteral("播放中") : QStringLiteral("已加载");
    emit nowPlayingChanged();
    setStatus(autoPlay ? QStringLiteral("播放中") : QStringLiteral("已加载文件"));

    if (autoPlay) {
        m_audioPlayer.play();
    }
}

void AppController::rebuildShuffleOrder()
{
    m_playbackController.setTrackCount(m_trackModel.rowCount());
    const int anchorRow = m_currentRow >= 0 ? m_currentRow : 0;
    m_playbackController.rebuildShuffleOrder(anchorRow);
}

void AppController::applyNavigateResult(const PlaybackNavigateResult& result, int currentRow)
{
    Q_UNUSED(currentRow);

    // 在线播放模式不接入本地列表的上一首/下一首
    if (m_isOnlinePlayback) {
        setStatus(QStringLiteral("播放完成"));
        m_currentSubtitle = QStringLiteral("播放完成");
        emit nowPlayingChanged();
        return;
    }

    if (result.replayCurrent && m_currentRow >= 0) {
        loadTrack(m_currentRow, true);
        return;
    }

    if (result.row < 0) {
        setStatus(QStringLiteral("播放完成"));
        m_currentSubtitle = QStringLiteral("播放完成");
        emit nowPlayingChanged();
        return;
    }

    playRow(result.row);
}

void AppController::updatePlaybackModeProperties()
{
    const PlaybackDisplayMode mode = playbackDisplayModeFromState(
        m_playbackController.state().shuffleEnabled,
        m_playbackController.state().repeatMode);
    m_playbackMode = static_cast<int>(mode);
    m_playbackModeTooltip = tooltipForMode(mode);
}

void AppController::setStatus(const QString& text)
{
    if (m_statusText == text) {
        return;
    }
    m_statusText = text;
    emit statusTextChanged();
}
