#include "app/AppController.h"

#include "media/TrackMetadataReader.h"
#include "network/GequbaoClient.h"
#include "network/MusicSourceClient.h"
#include "network/MusicSourceRegistry.h"
#include "network/MyFreeMp3Client.h"
#include "network/OnlineSongId.h"
#include "network/OnlineStreamLoader.h"
#include "network/StreamFetchOptions.h"

#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTimer>
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

QString normalizedSongId(const QString& songId)
{
    return OnlineSongId::normalizeLegacySongId(songId);
}

bool songIdsMatch(const QString& a, const QString& b)
{
    return normalizedSongId(a) == normalizedSongId(b);
}

QString sourceIdFromSongId(const QString& songId)
{
    QString src;
    if (OnlineSongId::parse(normalizedSongId(songId), &src, nullptr)) {
        return src;
    }
    return {};
}

bool sourcesMatch(const QString& expectedSource, const QString& actualSource)
{
    if (expectedSource.isEmpty() || actualSource.isEmpty()) {
        return true;
    }
    return expectedSource == actualSource;
}

QString effectiveSourceId(const QString& explicitSourceId, const QString& songId)
{
    if (!explicitSourceId.isEmpty()) {
        return explicitSourceId;
    }
    return sourceIdFromSongId(songId);
}

QString registrySourceLabel(const MusicSourceRegistry& registry,
                            const QString& explicitSourceId,
                            const QString& songId)
{
    const QString src = effectiveSourceId(explicitSourceId, songId);
    if (src.isEmpty()) {
        return {};
    }

    const QString label = registry.displayName(src);
    return label.isEmpty() ? src : label;
}

} // namespace

// 构造函数：初始化音量、播放模式，并连接 AudioPlayer 信号到 QML 属性通知
AppController::AppController(QObject* parent)
    : QObject(parent)
    , m_playbackController(this)
    , m_streamLoader(new OnlineStreamLoader(this))
{
    m_sourceRegistry.registerSource(new MyFreeMp3Client(this));
    m_sourceRegistry.registerSource(new GequbaoClient(this));
    m_musicSources = m_sourceRegistry.toVariantList();
    connectAllSourceClients();

    m_trackModel.setParent(this);
    m_searchResultModel.setParent(this);
    m_playlistTrackModel.setParent(this);
    m_playlistStore.setParent(this);
    m_audioPlayer.setParent(this);
    m_coverNetwork.setParent(this);

    m_audioPlayer.setVolume(m_volume);
    updatePlaybackModeProperties();

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

    // 一首播完：本地列表或在线队列切歌
    connect(&m_audioPlayer, &AudioPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::EndOfMedia) {
            return;
        }
        if (m_isOnlinePlayback) {
            playOnlineNext();
            return;
        }
        if (m_currentRow < 0) {
            return;
        }
        const PlaybackNavigateResult result = m_playbackController.navigateNext(m_currentRow);
        applyNavigateResult(result, m_currentRow);
    });

    connect(&m_playlistStore, &PlaylistStore::playlistsChanged, this, [this]() {
        syncSearchLikedStates();
        syncLocalLikedStates();
        syncPlaylistLikedStates();
        if (!m_activePlaylistId.isEmpty()) {
            reloadActivePlaylistModel();
        }
        emit playlistsChanged();
    });

    m_activePlaylistId = QStringLiteral("liked");
    reloadActivePlaylistModel();
    loadFavoriteFeaturedPlaylists();
    syncLocalLikedStates();
    syncPlaylistLikedStates();

    QTimer::singleShot(300, this, &AppController::prefetchFeaturedPlaylists);
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

PlaylistTrackModel* AppController::playlistTrackModel()
{
    return &m_playlistTrackModel;
}

int AppController::currentPage() const { return m_currentPage; }

void AppController::setCurrentPage(int page)
{
    if (m_currentPage == page) {
        if (page == 3 && !m_activePlaylistId.isEmpty()) {
            reloadActivePlaylistModel();
        }
        return;
    }
    m_currentPage = page;
    emit currentPageChanged();
    if (page == 3 && !m_activePlaylistId.isEmpty()) {
        reloadActivePlaylistModel();
    }
    if (page != 4 && !m_activeFeaturedPlaylistId.isEmpty() && !m_navigatingBack) {
        m_activeFeaturedPlaylistId.clear();
        emit activeFeaturedPlaylistChanged();
    }
}

bool AppController::isPlaying() const
{
    return m_audioPlayer.playbackState() == QMediaPlayer::PlayingState;
}

bool AppController::canControl() const { return m_canControl; }

QString AppController::currentTitle() const { return m_currentTitle; }

QString AppController::currentArtist() const { return m_currentArtist; }

QString AppController::currentSubtitle() const { return m_currentSubtitle; }

QString AppController::currentAlbum() const { return m_currentAlbum; }

QString AppController::currentSource() const { return m_currentSource; }
QString AppController::currentLyrics() const { return m_currentLyrics; }

QString AppController::currentSongId() const { return m_currentSongId; }

bool AppController::isOnlinePlayback() const { return m_isOnlinePlayback; }

bool AppController::isCurrentTrackLiked() const
{
    if (m_currentSongId.isEmpty()) {
        return false;
    }
    return m_playlistStore.containsTrack(QStringLiteral("liked"), m_currentSongId);
}

bool AppController::nowPlayingVisible() const { return m_nowPlayingVisible; }

void AppController::setNowPlayingVisible(bool visible)
{
    if (m_nowPlayingVisible == visible) {
        return;
    }
    m_nowPlayingVisible = visible;
    emit nowPlayingVisibleChanged();
}

void AppController::openNowPlaying()
{
    setNowPlayingVisible(true);
}

void AppController::closeNowPlaying()
{
    setNowPlayingVisible(false);
}

void AppController::updateCurrentLikeState()
{
    emit currentLikeChanged();
}

bool AppController::hasCover() const { return m_hasCover; }

QString AppController::currentCoverUrl() const
{
    return m_hasCover ? QStringLiteral("image://cover/current") : QString();
}

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

QVariantList AppController::sidebarPlaylists() const
{
    QVariantList items;
    for (const PlaylistInfo& info : m_playlistStore.playlists()) {
        QVariantMap map;
        map.insert(QStringLiteral("id"), info.id);
        map.insert(QStringLiteral("name"), info.name);
        map.insert(QStringLiteral("builtin"), info.builtin);
        map.insert(QStringLiteral("trackCount"), info.tracks.size());
        items.append(map);
    }
    return items;
}

QString AppController::activePlaylistId() const { return m_activePlaylistId; }

void AppController::setActivePlaylistId(const QString& id)
{
    const bool idChanged = m_activePlaylistId != id;
    if (idChanged) {
        m_activePlaylistId = id;
        emit activePlaylistIdChanged();
    }
    reloadActivePlaylistModel();
}

QString AppController::activePlaylistName() const
{
    const PlaylistInfo* playlist = m_playlistStore.playlistById(m_activePlaylistId);
    return playlist ? playlist->name : QString();
}

int AppController::activePlaylistTrackCount() const
{
    return m_playlistStore.trackCount(m_activePlaylistId);
}

int AppController::likedTrackCount() const
{
    return m_playlistStore.trackCount(QStringLiteral("liked"));
}

QVariantList AppController::featuredPlaylists() const
{
    QVariantList items;
    for (const FeaturedPlaylist& playlist : FeaturedPlaylistCatalog::all()) {
        QVariantMap map;
        map.insert(QStringLiteral("id"), playlist.id);
        map.insert(QStringLiteral("title"), playlist.title);
        map.insert(QStringLiteral("subtitle"), playlist.subtitle);
        map.insert(QStringLiteral("keyword"), playlist.keyword);
        map.insert(QStringLiteral("coverUrl"), playlist.coverUrl);
        map.insert(QStringLiteral("sourceId"), playlist.sourceId);
        items.append(map);
    }
    return items;
}

QVariantList AppController::favoriteFeaturedPlaylists() const
{
    QVariantList items;
    for (const QString& id : m_favoriteFeaturedPlaylistIds) {
        const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(id);
        if (!playlist) {
            continue;
        }
        QVariantMap map;
        map.insert(QStringLiteral("id"), playlist->id);
        map.insert(QStringLiteral("title"), playlist->title);
        map.insert(QStringLiteral("subtitle"), playlist->subtitle);
        map.insert(QStringLiteral("coverUrl"), playlist->coverUrl);
        items.append(map);
    }
    return items;
}

QString AppController::activeFeaturedPlaylistId() const
{
    return m_activeFeaturedPlaylistId;
}

QString AppController::activeFeaturedPlaylistTitle() const
{
    const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(m_activeFeaturedPlaylistId);
    return playlist ? playlist->title : QString();
}

QString AppController::activeFeaturedPlaylistSubtitle() const
{
    const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(m_activeFeaturedPlaylistId);
    return playlist ? playlist->subtitle : QString();
}

QString AppController::activeFeaturedPlaylistCoverUrl() const
{
    const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(m_activeFeaturedPlaylistId);
    return playlist ? playlist->coverUrl : QString();
}

bool AppController::activeFeaturedPlaylistFavorited() const
{
    return isFeaturedPlaylistFavorited(m_activeFeaturedPlaylistId);
}

bool AppController::isFeaturedPlaylistFavorited(const QString& id) const
{
    return !id.isEmpty() && m_favoriteFeaturedPlaylistIds.contains(id);
}

void AppController::loadFavoriteFeaturedPlaylists()
{
    QSettings settings;
    m_favoriteFeaturedPlaylistIds = settings.value(QStringLiteral("favoriteFeaturedPlaylists")).toStringList();
}

void AppController::saveFavoriteFeaturedPlaylists() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("favoriteFeaturedPlaylists"), m_favoriteFeaturedPlaylistIds);
}

void AppController::searchFeaturedPlaylist(const QString& keyword, int page)
{
    if (m_activeFeaturedPlaylistId.isEmpty()) {
        return;
    }

    const QString playlistId = m_activeFeaturedPlaylistId;
    if (hasFeaturedCache(playlistId, page)) {
        applyFeaturedCacheToUi(playlistId, page);
        return;
    }

    requestFeaturedSearch(playlistId, keyword, page, true);
}

bool AppController::hasFeaturedCache(const QString& playlistId, int page) const
{
    const auto playlistIt = m_featuredPlaylistCache.constFind(playlistId);
    if (playlistIt != m_featuredPlaylistCache.constEnd() && playlistIt->contains(page)) {
        return true;
    }
    return FeaturedPlaylistCacheStore::exists(playlistId, page);
}

const SearchPageResult* AppController::featuredCache(const QString& playlistId, int page) const
{
    const auto playlistIt = m_featuredPlaylistCache.constFind(playlistId);
    if (playlistIt == m_featuredPlaylistCache.constEnd()) {
        return nullptr;
    }
    const auto pageIt = playlistIt->constFind(page);
    if (pageIt == playlistIt->constEnd()) {
        return nullptr;
    }
    return &(*pageIt);
}

void AppController::storeFeaturedCache(const QString& playlistId,
                                       int page,
                                       const SearchPageResult& result)
{
    m_featuredPlaylistCache[playlistId][page] = result;
    FeaturedPlaylistCacheStore::save(playlistId, page, result);
}

void AppController::applyFeaturedCacheToUi(const QString& playlistId, int page)
{
    if (!featuredCache(playlistId, page)) {
        if (const auto loaded = FeaturedPlaylistCacheStore::load(playlistId, page)) {
            m_featuredPlaylistCache[playlistId][page] = *loaded;
        }
    }

    const SearchPageResult* cached = featuredCache(playlistId, page);
    if (!cached) {
        return;
    }

    const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(playlistId);
    if (playlist) {
        m_searchKeyword = playlist->keyword;
        emit searchKeywordChanged();
    }

    applySearchPageResult(*cached);
    setSearchBusy(false);
    setSearchStatus(featuredSearchStatusText(*cached));
}

QString AppController::featuredSearchStatusText(const SearchPageResult& result) const
{
    if (result.tracks.isEmpty()) {
        return QStringLiteral("暂无歌曲，请稍后再试");
    }
    return QStringLiteral("共 %1 首 · 第 %2 页")
        .arg(result.tracks.size())
        .arg(result.currentPage);
}

void AppController::requestFeaturedSearch(const QString& playlistId,
                                          const QString& keyword,
                                          int page,
                                          bool applyToUi)
{
    const QString trimmed = keyword.trimmed();
    if (trimmed.isEmpty() || playlistId.isEmpty()) {
        if (applyToUi) {
            setSearchStatus(QStringLiteral("歌单关键词无效"));
        }
        return;
    }

    if (m_featuredSearchInFlight) {
        if (applyToUi) {
            m_pendingFeaturedUiRequest = {playlistId, trimmed, page, true};
            m_hasPendingFeaturedUiRequest = true;
        } else if (!m_featuredPrefetchQueue.contains(playlistId)) {
            m_featuredPrefetchQueue.append(playlistId);
        }
        return;
    }

    m_featuredSearchInFlight = true;
    m_inFlightFeaturedSearch = {playlistId, trimmed, page, applyToUi};

    if (applyToUi) {
        m_searchKeyword = trimmed;
        emit searchKeywordChanged();
        setSearchBusy(true);
        setSearchStatus(QStringLiteral("正在从歌曲宝加载…"));
    }

    MusicSourceClient* client = m_sourceRegistry.source(QStringLiteral("gequbao"));
    if (!client) {
        m_featuredSearchInFlight = false;
        if (applyToUi) {
            setSearchBusy(false);
            setSearchStatus(QStringLiteral("未找到歌曲宝来源"));
        }
        startNextFeaturedPrefetch();
        return;
    }

    client->search(trimmed, page);
}

void AppController::prefetchFeaturedPlaylists()
{
    for (const FeaturedPlaylist& playlist : FeaturedPlaylistCatalog::all()) {
        if (hasFeaturedCache(playlist.id, 1)) {
            continue;
        }
        if (!m_featuredPrefetchQueue.contains(playlist.id)) {
            m_featuredPrefetchQueue.append(playlist.id);
        }
    }
    startNextFeaturedPrefetch();
}

void AppController::startNextFeaturedPrefetch()
{
    if (m_featuredSearchInFlight) {
        return;
    }

    while (!m_featuredPrefetchQueue.isEmpty()) {
        const QString playlistId = m_featuredPrefetchQueue.takeFirst();
        if (hasFeaturedCache(playlistId, 1)) {
            continue;
        }

        const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(playlistId);
        if (!playlist) {
            continue;
        }

        requestFeaturedSearch(playlistId, playlist->keyword, 1, false);
        return;
    }
}

bool AppController::canNavigateBack() const
{
    return !m_navBackStack.isEmpty();
}

void AppController::pushNavState()
{
    if (m_navigatingBack) {
        return;
    }

    NavSnapshot snapshot;
    snapshot.page = m_currentPage;
    snapshot.featuredPlaylistId = m_activeFeaturedPlaylistId;
    snapshot.playlistId = m_activePlaylistId;
    m_navBackStack.append(snapshot);
    emit canNavigateBackChanged();
}

void AppController::navigateToPage(int page)
{
    if (page == m_currentPage) {
        return;
    }

    pushNavState();

    if (page != 3) {
        setActivePlaylistId(QString());
    }

    setCurrentPage(page);
}

void AppController::navigateBack()
{
    if (m_navBackStack.isEmpty()) {
        return;
    }

    const NavSnapshot previous = m_navBackStack.takeLast();
    emit canNavigateBackChanged();

    m_navigatingBack = true;

    m_activeFeaturedPlaylistId = previous.featuredPlaylistId;
    emit activeFeaturedPlaylistChanged();

    if (m_activePlaylistId != previous.playlistId) {
        setActivePlaylistId(previous.playlistId);
    }

    setCurrentPage(previous.page);

    if (previous.page == 4 && !previous.featuredPlaylistId.isEmpty()) {
        if (hasFeaturedCache(previous.featuredPlaylistId, 1)) {
            applyFeaturedCacheToUi(previous.featuredPlaylistId, 1);
        } else {
            const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(previous.featuredPlaylistId);
            if (playlist) {
                searchFeaturedPlaylist(playlist->keyword, 1);
            }
        }
    }

    m_navigatingBack = false;
}


QString AppController::activeMusicSourceId() const
{
    return m_activeMusicSourceId;
}

void AppController::setActiveMusicSourceId(const QString& id)
{
    if (m_activeMusicSourceId == id || id.isEmpty()) {
        return;
    }
    if (!m_sourceRegistry.source(id)) {
        return;
    }

    m_activeMusicSourceId = id;
    m_searchResultModel.setResults({});
    m_searchCurrentPage = 1;
    m_searchHasNext = false;
    m_searchHasPrevious = false;
    emit searchPaginationChanged();
    emit searchResultCountChanged();
    setSearchStatus(QStringLiteral("已切换至「%1」，请输入关键词搜索")
                        .arg(m_sourceRegistry.displayName(id)));
    emit activeMusicSourceChanged();
}

QString AppController::activeMusicSourceName() const
{
    return m_sourceRegistry.displayName(m_activeMusicSourceId);
}

QVariantList AppController::musicSources() const
{
    return m_musicSources;
}

void AppController::connectAllSourceClients()
{
    for (const QString& sourceId : m_sourceRegistry.sourceIds()) {
        MusicSourceClient* client = m_sourceRegistry.source(sourceId);
        if (!client) {
            continue;
        }

        connect(client, &MusicSourceClient::searchCompleted,
                this, &AppController::onSearchCompleted);
        connect(client, &MusicSourceClient::searchFailed,
                this, &AppController::onSearchFailed);
        connect(client, &MusicSourceClient::streamUrlResolved,
                this, &AppController::onStreamUrlResolved);
        connect(client, &MusicSourceClient::streamUrlFailed,
                this, &AppController::onStreamUrlFailed);
    }
}

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
    if (MusicSourceClient* client = m_sourceRegistry.source(m_activeMusicSourceId)) {
        client->search(trimmed, page);
    } else {
        setSearchBusy(false);
        setSearchStatus(QStringLiteral("未找到音乐来源「%1」").arg(m_activeMusicSourceId));
    }
}

void AppController::searchNextPage()
{
    if (!m_searchHasNext) {
        return;
    }
    if (!m_activeFeaturedPlaylistId.isEmpty()) {
        const QString keyword = activeFeaturedKeyword();
        if (keyword.isEmpty()) {
            return;
        }
        searchFeaturedPlaylist(keyword, m_searchCurrentPage + 1);
        return;
    }
    if (m_searchKeyword.isEmpty()) {
        return;
    }
    searchOnline(m_searchKeyword, m_searchCurrentPage + 1);
}

void AppController::searchPreviousPage()
{
    if (!m_searchHasPrevious) {
        return;
    }
    if (!m_activeFeaturedPlaylistId.isEmpty()) {
        const QString keyword = activeFeaturedKeyword();
        if (keyword.isEmpty()) {
            return;
        }
        searchFeaturedPlaylist(keyword, m_searchCurrentPage - 1);
        return;
    }
    if (m_searchKeyword.isEmpty()) {
        return;
    }
    searchOnline(m_searchKeyword, m_searchCurrentPage - 1);
}

QString AppController::activeFeaturedKeyword() const
{
    const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(m_activeFeaturedPlaylistId);
    return playlist ? playlist->keyword : QString();
}

void AppController::applySearchPageResult(const SearchPageResult& result)
{
    QVector<SearchResultEntry> entries;
    entries.reserve(result.tracks.size());
    for (const OnlineTrack& track : result.tracks) {
        SearchResultEntry entry;
        entry.songId = track.songId;
        entry.sourceId = track.sourceId.isEmpty()
            ? ((!m_activeFeaturedPlaylistId.isEmpty() && m_currentPage == 4)
                   ? QStringLiteral("gequbao")
                   : m_activeMusicSourceId)
            : track.sourceId;
        entry.sourceLabel = m_sourceRegistry.displayName(entry.sourceId);
        entry.detailUrl = track.detailUrl;
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
    syncSearchLikedStates();
    emit searchPaginationChanged();
    emit searchResultCountChanged();
    queueSearchStreamPrefetch();
}

void AppController::onSearchCompleted(const SearchPageResult& result, const QString& keyword)
{
    MusicSourceClient* client = qobject_cast<MusicSourceClient*>(sender());
    const QString sourceId = client ? client->sourceId() : QString();

    if (sourceId == QStringLiteral("gequbao") && m_featuredSearchInFlight) {
        const FeaturedSearchRequest request = m_inFlightFeaturedSearch;
        m_featuredSearchInFlight = false;

        storeFeaturedCache(request.playlistId, request.page, result);

        if (request.applyToUi
            && m_activeFeaturedPlaylistId == request.playlistId
            && m_currentPage == 4) {
            applySearchPageResult(result);
            setSearchBusy(false);
            if (result.tracks.isEmpty()) {
                setSearchStatus(QStringLiteral("未找到「%1」的相关歌曲").arg(keyword));
            } else {
                setSearchStatus(QStringLiteral("找到 %1 首 · 第 %2 页")
                                    .arg(result.tracks.size())
                                    .arg(result.currentPage));
            }
        } else if (!request.applyToUi
                   && m_activeFeaturedPlaylistId == request.playlistId
                   && m_currentPage == 4) {
            applyFeaturedCacheToUi(request.playlistId, request.page);
        }

        if (m_hasPendingFeaturedUiRequest) {
            const FeaturedSearchRequest pending = m_pendingFeaturedUiRequest;
            m_hasPendingFeaturedUiRequest = false;
            m_featuredSearchInFlight = false;
            if (hasFeaturedCache(pending.playlistId, pending.page)) {
                if (pending.applyToUi
                    && m_activeFeaturedPlaylistId == pending.playlistId
                    && m_currentPage == 4) {
                    applyFeaturedCacheToUi(pending.playlistId, pending.page);
                }
                startNextFeaturedPrefetch();
            } else {
                requestFeaturedSearch(pending.playlistId,
                                      pending.keyword,
                                      pending.page,
                                      pending.applyToUi);
            }
            return;
        }

        startNextFeaturedPrefetch();
        return;
    }

    if (client && client->sourceId() != m_activeMusicSourceId) {
        return;
    }

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
    MusicSourceClient* client = qobject_cast<MusicSourceClient*>(sender());
    const QString sourceId = client ? client->sourceId() : QString();

    if (sourceId == QStringLiteral("gequbao") && m_featuredSearchInFlight) {
        const FeaturedSearchRequest request = m_inFlightFeaturedSearch;
        m_featuredSearchInFlight = false;

        if (request.applyToUi
            && m_activeFeaturedPlaylistId == request.playlistId
            && m_currentPage == 4) {
            setSearchBusy(false);
            setSearchStatus(QStringLiteral("加载失败：%1").arg(message));
        }

        if (m_hasPendingFeaturedUiRequest) {
            const FeaturedSearchRequest pending = m_pendingFeaturedUiRequest;
            m_hasPendingFeaturedUiRequest = false;
            m_featuredSearchInFlight = false;
            if (hasFeaturedCache(pending.playlistId, pending.page)) {
                if (pending.applyToUi
                    && m_activeFeaturedPlaylistId == pending.playlistId
                    && m_currentPage == 4) {
                    applyFeaturedCacheToUi(pending.playlistId, pending.page);
                }
                startNextFeaturedPrefetch();
            } else {
                requestFeaturedSearch(pending.playlistId,
                                      pending.keyword,
                                      pending.page,
                                      pending.applyToUi);
            }
            return;
        }

        startNextFeaturedPrefetch();
        return;
    }

    if (client && client->sourceId() != m_activeMusicSourceId) {
        return;
    }

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
    m_onlineQueueType = OnlineQueueType::Search;
    m_onlineQueueRow = row;
    m_pendingSearchRow = row;
    m_pendingPlaylistRow = -1;
    if (!m_canControl) {
        m_canControl = true;
        emit canControlChanged();
    }
    m_searchResultModel.setPlayingRow(row);
    m_searchResultModel.setSelectedRow(row);
    m_playlistTrackModel.setPlayingRow(-1);

    const QVector<SearchResultEntry>& entries = m_searchResultModel.entries();
    const SearchResultEntry& entry = entries.at(row);
    m_currentTitle = entry.metadata.title;
    m_currentArtist = entry.metadata.artist.isEmpty()
        ? QStringLiteral("在线音乐")
        : entry.metadata.artist;
    m_currentAlbum = entry.metadata.album.isEmpty()
        ? QStringLiteral("未知专辑")
        : entry.metadata.album;
    m_currentSource = registrySourceLabel(m_sourceRegistry, entry.sourceId, entry.songId);
    m_currentSongId = entry.songId;
    m_currentLocalPath.clear();
    m_currentLyrics = entry.lyrics;

    const QString cachedUrl = entry.streamUrl;
    if (!cachedUrl.isEmpty()) {
        beginOnlinePlaybackUi(QStringLiteral("缓冲中…"));
        if (!entry.coverUrl.isEmpty()) {
            downloadNowPlayingCover(QUrl(entry.coverUrl), entry.songId);
        }
        loadOnlineStream(cachedUrl, m_currentTitle, m_currentArtist, {}, true, entry.lyrics);
        return;
    }

    beginOnlinePlaybackUi(QStringLiteral("正在获取播放地址…"));
    cancelStreamUrlPrefetch();
    for (const QString& sourceId : m_sourceRegistry.sourceIds()) {
        if (MusicSourceClient* client = m_sourceRegistry.source(sourceId)) {
            client->cancelResolveStreamUrl();
        }
    }

    setSearchBusy(true);
    setSearchStatus(QStringLiteral("正在获取播放地址…"));
    const QString src = effectiveSourceId(entry.sourceId, entry.songId);
    if (MusicSourceClient* client = m_sourceRegistry.source(src)) {
        client->resolveStreamUrl(entry.songId,
                                 QUrl(entry.detailUrl),
                                 entry.metadata.title,
                                 entry.metadata.artist);
    } else {
        handleOnlinePlaybackFailure(QStringLiteral("未找到音乐来源「%1」").arg(src));
    }
}

void AppController::onStreamUrlResolved(const OnlineTrack& track)
{
    if (m_activeStreamPrefetchRow >= 0) {
        const int row = m_activeStreamPrefetchRow;
        m_streamPrefetchInFlight = false;
        m_activeStreamPrefetchRow = -1;

        if (row >= 0 && row < m_searchResultModel.rowCount()) {
            const SearchResultEntry& entry = m_searchResultModel.entries().at(row);
            if (songIdsMatch(entry.songId, track.songId)
                && sourcesMatch(effectiveSourceId(entry.sourceId, entry.songId),
                                 effectiveSourceId(track.sourceId, track.songId))) {
                m_searchResultModel.updateStreamUrl(row, track.streamUrl, track.coverUrl, track.lyrics);
                m_playlistStore.updateTrackStreamUrl(entry.songId, track.streamUrl, track.coverUrl);
            }
        }

        startNextStreamPrefetch();
        if (m_pendingSearchRow < 0 && m_pendingPlaylistRow < 0) {
            return;
        }
    }

    setSearchBusy(false);

    if (m_pendingSearchRow >= 0) {
        if (!isPendingOnlineSearchRow(m_pendingSearchRow)) {
            return;
        }

        const SearchResultEntry& entry = m_searchResultModel.entries().at(m_pendingSearchRow);
        if (!songIdsMatch(entry.songId, track.songId)) {
            return;
        }
        if (!sourcesMatch(effectiveSourceId(entry.sourceId, entry.songId),
                          effectiveSourceId(track.sourceId, track.songId))) {
            return;
        }

        m_searchResultModel.updateStreamUrl(m_pendingSearchRow, track.streamUrl, track.coverUrl, track.lyrics);
        const QString songId = m_searchResultModel.songIdAt(m_pendingSearchRow);
        m_playlistStore.updateTrackStreamUrl(songId, track.streamUrl, track.coverUrl);

        const QString title = track.title.isEmpty() ? track.displayTitle : track.title;
        const QString artist = track.artist.isEmpty() ? QStringLiteral("在线音乐") : track.artist;
        m_currentTitle = title;
        m_currentArtist = artist;
        m_currentLyrics = track.lyrics;
        setSearchStatus(QStringLiteral("正在播放…"));
        if (!track.coverUrl.isEmpty()) {
            downloadNowPlayingCover(QUrl(track.coverUrl), track.songId);
        }
        loadOnlineStream(track.streamUrl, title, artist, {}, true, track.lyrics);
        return;
    }

    if (m_pendingPlaylistRow >= 0) {
        if (!isPendingOnlinePlaylistRow(m_pendingPlaylistRow)) {
            return;
        }

        const PlaylistTrackRef& ref = m_playlistTrackModel.entries().at(m_pendingPlaylistRow).ref;
        if (!songIdsMatch(ref.songId, track.songId)) {
            return;
        }
        if (!sourcesMatch(effectiveSourceId(ref.sourceId, ref.songId),
                          effectiveSourceId(track.sourceId, track.songId))) {
            return;
        }

        const QString title = track.title.isEmpty() ? track.displayTitle : track.title;
        const QString artist = track.artist.isEmpty() ? QStringLiteral("在线音乐") : track.artist;
        m_currentTitle = title;
        m_currentArtist = artist;
        m_currentLyrics = track.lyrics;
        setSearchStatus(QStringLiteral("正在播放…"));
        if (!track.coverUrl.isEmpty()) {
            downloadNowPlayingCover(QUrl(track.coverUrl), track.songId);
        }
        loadOnlineStream(track.streamUrl, title, artist, {}, true, track.lyrics);
    }
}

void AppController::beginOnlinePlaybackUi(const QString& subtitle)
{
    m_currentSubtitle = subtitle;
    m_currentCover = {};
    m_hasCover = false;
    emit nowPlayingChanged();
}

void AppController::downloadNowPlayingCover(const QUrl& coverUrl, const QString& songId)
{
    if (!coverUrl.isValid()) {
        return;
    }

    const QString expectedSongId = songId.isEmpty() ? m_currentSongId : songId;

    QNetworkRequest request(coverUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Mozilla/5.0 MusicQuick/1.0"));
    request.setTransferTimeout(10000);

    QNetworkReply* reply = m_coverNetwork.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, expectedSongId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }
        if (m_currentSongId != expectedSongId) {
            return;
        }
        QImage image;
        if (!image.loadFromData(reply->readAll())) {
            return;
        }
        m_currentCover = image;
        m_hasCover = true;
        emit nowPlayingChanged();
    });
}

void AppController::queueSearchStreamPrefetch()
{
    cancelStreamUrlPrefetch();

    const QVector<SearchResultEntry>& entries = m_searchResultModel.entries();
    const int limit = qMin(entries.size(), kStreamPrefetchCount);
    for (int row = 0; row < limit; ++row) {
        if (entries.at(row).streamUrl.isEmpty()) {
            m_streamPrefetchQueue.append(row);
        }
    }

    if (!m_streamPrefetchQueue.isEmpty()) {
        QTimer::singleShot(400, this, &AppController::startNextStreamPrefetch);
    }
}

void AppController::startNextStreamPrefetch()
{
    if (m_streamPrefetchInFlight || m_pendingSearchRow >= 0 || m_pendingPlaylistRow >= 0) {
        return;
    }

    while (!m_streamPrefetchQueue.isEmpty()) {
        const int row = m_streamPrefetchQueue.takeFirst();
        if (row < 0 || row >= m_searchResultModel.rowCount()) {
            continue;
        }

        const SearchResultEntry& entry = m_searchResultModel.entries().at(row);
        if (!entry.streamUrl.isEmpty()) {
            continue;
        }

        const QString src = effectiveSourceId(entry.sourceId, entry.songId);
        MusicSourceClient* client = m_sourceRegistry.source(src);
        if (!client) {
            continue;
        }

        m_streamPrefetchInFlight = true;
        m_activeStreamPrefetchRow = row;
        client->resolveStreamUrl(entry.songId,
                                 QUrl(entry.detailUrl),
                                 entry.metadata.title,
                                 entry.metadata.artist);
        return;
    }
}

void AppController::cancelStreamUrlPrefetch()
{
    m_streamPrefetchQueue.clear();
    m_streamPrefetchInFlight = false;
    m_activeStreamPrefetchRow = -1;
}

void AppController::onStreamUrlFailed(const QString& songId, const QString& message)
{
    if (m_activeStreamPrefetchRow >= 0) {
        m_streamPrefetchInFlight = false;
        m_activeStreamPrefetchRow = -1;
        startNextStreamPrefetch();
        if (m_pendingSearchRow < 0 && m_pendingPlaylistRow < 0) {
            return;
        }
    }

    if (m_pendingSearchRow >= 0) {
        if (!isPendingOnlineSearchRow(m_pendingSearchRow)) {
            return;
        }
        const SearchResultEntry& entry = m_searchResultModel.entries().at(m_pendingSearchRow);
        if (!songIdsMatch(entry.songId, songId)) {
            return;
        }
        if (!sourcesMatch(effectiveSourceId(entry.sourceId, entry.songId),
                          sourceIdFromSongId(songId))) {
            return;
        }
    } else if (m_pendingPlaylistRow >= 0) {
        if (!isPendingOnlinePlaylistRow(m_pendingPlaylistRow)) {
            return;
        }
        const PlaylistTrackRef& ref = m_playlistTrackModel.entries().at(m_pendingPlaylistRow).ref;
        if (!songIdsMatch(ref.songId, songId)) {
            return;
        }
        if (!sourcesMatch(effectiveSourceId(ref.sourceId, ref.songId),
                          sourceIdFromSongId(songId))) {
            return;
        }
    } else {
        return;
    }

    handleOnlinePlaybackFailure(QStringLiteral("解析失败：%1").arg(message));
}

void AppController::loadOnlineStream(const QString& streamUrl,
                                     const QString& title,
                                     const QString& artist,
                                     const QImage& cover,
                                     bool autoPlay,
                                     const QString& lyrics)
{
    if (streamUrl.isEmpty() || !m_streamLoader) {
        return;
    }

    for (const QString& sourceId : m_sourceRegistry.sourceIds()) {
        if (MusicSourceClient* client = m_sourceRegistry.source(sourceId)) {
            client->cancelResolveStreamUrl();
        }
    }

    const QString streamSourceId = effectiveSourceId(QString(), m_currentSongId);
    if (MusicSourceClient* client = m_sourceRegistry.source(
            streamSourceId.isEmpty() ? m_activeMusicSourceId : streamSourceId)) {
        m_currentStreamFetchOptions = client->streamFetchOptions();
    }

    // CDN 拒绝无 Referer 的直接串流（403），先预取到本地缓存再交给 QMediaPlayer
    m_isOnlinePlayback = true;
    m_currentRow = -1;
    m_currentFilePath = streamUrl;
    m_currentTitle = title;
    m_currentArtist = artist;
    m_currentLyrics = lyrics;
    m_currentCover = cover;
    m_hasCover = !cover.isNull();
    m_pendingStreamAutoPlay = autoPlay;
    m_pendingStreamUrl = streamUrl;

    if (!m_canControl) {
        m_canControl = true;
        emit canControlChanged();
    }

    m_currentSubtitle = QStringLiteral("缓冲中…");
    emit nowPlayingChanged();
    setStatus(QStringLiteral("正在缓冲在线音频…"));
    setSearchStatus(QStringLiteral("正在缓冲…"));

    m_streamLoader->prefetch(QUrl(streamUrl), m_currentStreamFetchOptions);
}

void AppController::onStreamPrefetchReady(const QString& localFilePath, const QUrl& originalUrl)
{
    if (m_pendingStreamUrl.isEmpty() || originalUrl.toString() != m_pendingStreamUrl) {
        return;
    }

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
    if (m_pendingStreamUrl.isEmpty() || originalUrl.toString() != m_pendingStreamUrl) {
        return;
    }

    handleOnlinePlaybackFailure(QStringLiteral("缓冲失败：%1").arg(message));
}

void AppController::handleOnlinePlaybackFailure(const QString& message)
{
    cancelPendingOnlineRequests();
    setSearchBusy(false);
    setSearchStatus(message);
    setStatus(message);
    m_currentSubtitle = QStringLiteral("解析失败");
    m_pendingSearchRow = -1;
    m_pendingPlaylistRow = -1;
    if (!m_canControl && m_onlineQueueType != OnlineQueueType::None) {
        m_canControl = true;
        emit canControlChanged();
    }
    emit nowPlayingChanged();
}

void AppController::cancelPendingOnlineRequests()
{
    cancelStreamUrlPrefetch();
    m_pendingStreamUrl.clear();
    if (m_streamLoader) {
        m_streamLoader->cancelActivePrefetch();
    }
    for (const QString& sourceId : m_sourceRegistry.sourceIds()) {
        if (MusicSourceClient* client = m_sourceRegistry.source(sourceId)) {
            client->cancelResolveStreamUrl();
        }
    }
}

bool AppController::isPendingOnlineSearchRow(int row) const
{
    return m_onlineQueueType == OnlineQueueType::Search && m_onlineQueueRow == row;
}

bool AppController::isPendingOnlinePlaylistRow(int row) const
{
    return m_onlineQueueType == OnlineQueueType::Playlist && m_onlineQueueRow == row;
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
    syncLocalLikedStates();

    rebuildShuffleOrder();
    m_trackModel.setPlayingRow(0);
    m_trackModel.setSelectedRow(0);
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

    cancelPendingOnlineRequests();
    m_isOnlinePlayback = false;
    m_onlineQueueType = OnlineQueueType::None;
    m_pendingSearchRow = -1;
    m_pendingPlaylistRow = -1;
    m_searchResultModel.setPlayingRow(-1);
    m_playlistTrackModel.setPlayingRow(-1);

    m_playbackController.syncShuffleIndexForRow(row);
    m_trackModel.setPlayingRow(row);
    m_trackModel.setSelectedRow(row);
    loadTrack(row, true);
}

void AppController::selectLocalRow(int row)
{
    if (row < 0 || row >= m_trackModel.rowCount()) {
        return;
    }
    m_trackModel.setSelectedRow(row);
}

void AppController::selectSearchRow(int row)
{
    if (row < 0 || row >= m_searchResultModel.rowCount()) {
        return;
    }
    m_searchResultModel.setSelectedRow(row);
}

void AppController::selectPlaylistRow(int row)
{
    if (row < 0 || row >= m_playlistTrackModel.rowCount()) {
        return;
    }
    m_playlistTrackModel.setSelectedRow(row);
}

void AppController::playAllLocal()
{
    if (m_trackModel.rowCount() <= 0) {
        return;
    }
    const int row = m_trackModel.selectedRow() >= 0 ? m_trackModel.selectedRow() : 0;
    playRow(row);
}

void AppController::toggleLikeLocalRow(int row)
{
    const PlaylistTrackRef ref = trackRefFromLocalRow(row);
    if (ref.songId.isEmpty()) {
        return;
    }

    const bool liked = m_playlistStore.containsTrack(QStringLiteral("liked"), ref.songId);
    if (liked) {
        m_playlistStore.removeTrack(QStringLiteral("liked"), ref.songId);
        m_trackModel.refreshLikedState(row, false);
        setStatus(QStringLiteral("已从「我喜欢的音乐」移除"));
        return;
    }

    if (m_playlistStore.addTrack(QStringLiteral("liked"), ref)) {
        m_trackModel.refreshLikedState(row, true);
        setStatus(QStringLiteral("已加入「我喜欢的音乐」"));
    }
}

PlaylistTrackRef AppController::trackRefFromLocalRow(int row) const
{
    PlaylistTrackRef ref;
    if (row < 0 || row >= m_trackModel.rowCount()) {
        return ref;
    }
    const TrackEntry& entry = m_trackModel.entries().at(row);
    ref.songId = PlaylistStore::localSongId(entry.filePath);
    ref.localPath = entry.filePath;
    ref.title = entry.metadata.title;
    ref.artist = entry.metadata.artist;
    ref.album = entry.metadata.album;
    ref.streamUrl = entry.filePath;
    return ref;
}

void AppController::syncLocalLikedStates()
{
    QSet<QString> likedIds;
    const PlaylistInfo* liked = m_playlistStore.playlistById(QStringLiteral("liked"));
    if (liked) {
        for (const PlaylistTrackRef& track : liked->tracks) {
            if (PlaylistStore::isLocalSongId(track.songId) || !track.localPath.isEmpty()) {
                likedIds.insert(track.songId.isEmpty()
                    ? PlaylistStore::localSongId(track.localPath)
                    : track.songId);
            }
        }
    }
    m_trackModel.setLikedSongIds(likedIds);
}

void AppController::playNext()
{
    if (m_onlineQueueType != OnlineQueueType::None) {
        playOnlineNext();
        return;
    }
    if (m_currentRow < 0) {
        return;
    }

    const PlaybackNavigateResult result = m_playbackController.navigateNext(m_currentRow);
    applyNavigateResult(result, m_currentRow);
}

void AppController::playPrevious()
{
    if (m_onlineQueueType != OnlineQueueType::None) {
        playOnlinePrevious();
        return;
    }
    if (m_currentRow < 0) {
        return;
    }

    const PlaybackNavigateResult result = m_playbackController.navigatePrevious(m_currentRow);
    applyNavigateResult(result, m_currentRow);
}

void AppController::playOnlineNext()
{
    if (m_onlineQueueType == OnlineQueueType::Search) {
        const int next = m_onlineQueueRow + 1;
        if (next < m_searchResultModel.rowCount()) {
            playSearchRow(next);
        } else {
            setStatus(QStringLiteral("已是最后一首"));
        }
        return;
    }
    if (m_onlineQueueType == OnlineQueueType::Playlist) {
        const int next = m_onlineQueueRow + 1;
        if (next < m_playlistTrackModel.rowCount()) {
            playPlaylistRow(next);
        } else {
            setStatus(QStringLiteral("已是最后一首"));
        }
    }
}

void AppController::playOnlinePrevious()
{
    if (m_onlineQueueType == OnlineQueueType::Search) {
        const int prev = m_onlineQueueRow - 1;
        if (prev >= 0) {
            playSearchRow(prev);
        } else {
            setStatus(QStringLiteral("已是第一首"));
        }
        return;
    }
    if (m_onlineQueueType == OnlineQueueType::Playlist) {
        const int prev = m_onlineQueueRow - 1;
        if (prev >= 0) {
            playPlaylistRow(prev);
        } else {
            setStatus(QStringLiteral("已是第一首"));
        }
    }
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
        m_currentAlbum = entry.metadata.album.isEmpty()
            ? QStringLiteral("未知专辑")
            : entry.metadata.album;
        m_currentCover = entry.metadata.cover;
        m_hasCover = !entry.metadata.cover.isNull();
        m_currentSource = QStringLiteral("本地音乐");
        m_currentSongId = PlaylistStore::localSongId(path);
        m_currentLocalPath = path;
    } else {
        const QFileInfo info(path);
        m_currentTitle = info.fileName();
        m_currentArtist = QStringLiteral("未知艺术家");
        m_currentAlbum = QStringLiteral("未知专辑");
        m_currentCover = {};
        m_hasCover = false;
        m_currentSource = QStringLiteral("本地音乐");
        m_currentSongId = PlaylistStore::localSongId(path);
        m_currentLocalPath = path;
    }
    m_currentLyrics.clear();

    if (!m_canControl) {
        m_canControl = true;
        emit canControlChanged();
    }

    m_currentSubtitle = autoPlay ? QStringLiteral("播放中") : QStringLiteral("已加载");
    emit nowPlayingChanged();
    updateCurrentLikeState();
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

    // 在线播放模式：EndOfMedia 已在 mediaStatusChanged 中处理
    if (m_isOnlinePlayback) {
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

void AppController::syncSearchLikedStates()
{
    QSet<QString> likedIds;
    const PlaylistInfo* liked = m_playlistStore.playlistById(QStringLiteral("liked"));
    if (liked) {
        for (const PlaylistTrackRef& track : liked->tracks) {
            likedIds.insert(track.songId);
        }
    }
    m_searchResultModel.setLikedSongIds(likedIds);
}

void AppController::syncPlaylistLikedStates()
{
    QSet<QString> likedIds;
    const PlaylistInfo* liked = m_playlistStore.playlistById(QStringLiteral("liked"));
    if (liked) {
        for (const PlaylistTrackRef& track : liked->tracks) {
            likedIds.insert(track.songId);
        }
    }
    m_playlistTrackModel.setLikedSongIds(likedIds);
}

void AppController::reloadActivePlaylistModel()
{
    if (m_activePlaylistId.isEmpty()) {
        return;
    }

    const PlaylistInfo* playlist = m_playlistStore.playlistById(m_activePlaylistId);
    if (!playlist) {
        m_playlistTrackModel.setTracks({});
        emit activePlaylistContentChanged();
        return;
    }

    QVector<PlaylistTrackEntry> entries;
    entries.reserve(playlist->tracks.size());
    for (const PlaylistTrackRef& ref : playlist->tracks) {
        PlaylistTrackEntry entry;
        entry.ref = ref;
        entry.metadata.title = ref.title;
        entry.metadata.artist = ref.artist;
        entry.metadata.album = ref.album.isEmpty() ? QStringLiteral("在线音乐") : ref.album;
        entry.metadata.durationText = QStringLiteral("--:--");
        entries.append(entry);
    }
    m_playlistTrackModel.setTracks(std::move(entries));
    syncPlaylistLikedStates();
    emit activePlaylistContentChanged();
}

void AppController::refreshActivePlaylist()
{
    reloadActivePlaylistModel();
}

PlaylistTrackRef AppController::trackRefFromSearchRow(int row) const
{
    PlaylistTrackRef ref;
    if (row < 0 || row >= m_searchResultModel.rowCount()) {
        return ref;
    }
    const SearchResultEntry& entry = m_searchResultModel.entries().at(row);
    ref.songId = entry.songId;
    ref.sourceId = entry.sourceId;
    if (ref.sourceId.isEmpty()) {
        QString src;
        if (OnlineSongId::parse(ref.songId, &src, nullptr)) {
            ref.sourceId = src;
        }
    }
    ref.title = entry.metadata.title;
    ref.artist = entry.metadata.artist;
    ref.album = entry.metadata.album;
    ref.detailUrl = entry.detailUrl;
    ref.streamUrl = entry.streamUrl;
    ref.coverUrl = entry.coverUrl;
    return ref;
}

void AppController::playOnlineTrackRef(const PlaylistTrackRef& track, bool autoPlay)
{
    m_isOnlinePlayback = true;
    m_currentRow = -1;
    m_currentTitle = track.title;
    m_currentArtist = track.artist.isEmpty() ? QStringLiteral("在线音乐") : track.artist;
    m_currentSongId = track.songId;
    m_currentSource = registrySourceLabel(m_sourceRegistry, track.sourceId, track.songId);

    if (!track.streamUrl.isEmpty()) {
        beginOnlinePlaybackUi(QStringLiteral("缓冲中…"));
        if (!track.coverUrl.isEmpty()) {
            downloadNowPlayingCover(QUrl(track.coverUrl), track.songId);
        }
        loadOnlineStream(track.streamUrl, m_currentTitle, m_currentArtist, {}, autoPlay);
        return;
    }

    beginOnlinePlaybackUi(QStringLiteral("正在获取播放地址…"));
    cancelStreamUrlPrefetch();
    for (const QString& sourceId : m_sourceRegistry.sourceIds()) {
        if (MusicSourceClient* client = m_sourceRegistry.source(sourceId)) {
            client->cancelResolveStreamUrl();
        }
    }

    setSearchBusy(true);
    setSearchStatus(QStringLiteral("正在获取播放地址…"));
    const QString src = effectiveSourceId(track.sourceId, track.songId);
    if (MusicSourceClient* client = m_sourceRegistry.source(src)) {
        client->resolveStreamUrl(track.songId,
                                 QUrl(track.detailUrl),
                                 track.title,
                                 track.artist);
    } else {
        handleOnlinePlaybackFailure(QStringLiteral("未找到音乐来源「%1」").arg(src));
    }
}

void AppController::playAllSearchResults()
{
    if (m_searchResultModel.rowCount() <= 0) {
        return;
    }
    playSearchRow(0);
}

void AppController::likeAllSearchResults()
{
    int added = 0;
    for (int i = 0; i < m_searchResultModel.rowCount(); ++i) {
        const PlaylistTrackRef ref = trackRefFromSearchRow(i);
        if (ref.songId.isEmpty()) {
            continue;
        }
        if (m_playlistStore.addTrack(QStringLiteral("liked"), ref)) {
            m_searchResultModel.refreshLikedState(i, true);
            ++added;
        }
    }
    if (added > 0) {
        setSearchStatus(QStringLiteral("已收藏 %1 首至「我喜欢的音乐」").arg(added));
    }
}

bool AppController::isSearchRowLiked(int row) const
{
    const PlaylistTrackRef ref = trackRefFromSearchRow(row);
    if (ref.songId.isEmpty()) {
        return false;
    }
    return m_playlistStore.containsTrack(QStringLiteral("liked"), ref.songId);
}

void AppController::toggleLikeSearchRow(int row)
{
    const PlaylistTrackRef ref = trackRefFromSearchRow(row);
    if (ref.songId.isEmpty()) {
        return;
    }

    const bool liked = m_playlistStore.containsTrack(QStringLiteral("liked"), ref.songId);
    if (liked) {
        m_playlistStore.removeTrack(QStringLiteral("liked"), ref.songId);
        m_searchResultModel.refreshLikedState(row, false);
        syncPlaylistLikedStates();
        syncLocalLikedStates();
        setSearchStatus(QStringLiteral("已取消喜欢"));
        return;
    }

    if (m_playlistStore.addTrack(QStringLiteral("liked"), ref)) {
        m_searchResultModel.refreshLikedState(row, true);
        syncPlaylistLikedStates();
        syncLocalLikedStates();
        setSearchStatus(QStringLiteral("已加入「我喜欢的音乐」"));
    }
}

void AppController::addSearchRowToPlaylist(int row, const QString& playlistId)
{
    const PlaylistTrackRef ref = trackRefFromSearchRow(row);
    if (ref.songId.isEmpty() || playlistId.isEmpty()) {
        return;
    }

    if (m_playlistStore.addTrack(playlistId, ref)) {
        const PlaylistInfo* playlist = m_playlistStore.playlistById(playlistId);
        const QString name = playlist ? playlist->name : QStringLiteral("歌单");
        setSearchStatus(QStringLiteral("已加入「%1」").arg(name));
        if (playlistId == QStringLiteral("liked")) {
            m_searchResultModel.refreshLikedState(row, true);
        }
    }
}

QString AppController::createPlaylist(const QString& name)
{
    const QString id = m_playlistStore.createPlaylist(name);
    if (!id.isEmpty()) {
        setStatus(QStringLiteral("已创建歌单「%1」").arg(name.trimmed()));
    }
    return id;
}

bool AppController::deletePlaylist(const QString& id)
{
    const PlaylistInfo* playlist = m_playlistStore.playlistById(id);
    if (!playlist || playlist->builtin) {
        return false;
    }
    const QString name = playlist->name;
    if (!m_playlistStore.deletePlaylist(id)) {
        return false;
    }
    if (m_activePlaylistId == id) {
        setActivePlaylistId(QStringLiteral("liked"));
        setCurrentPage(3);
    }
    setStatus(QStringLiteral("已删除歌单「%1」").arg(name));
    return true;
}

void AppController::openPlaylist(const QString& id)
{
    if (!m_playlistStore.playlistById(id)) {
        return;
    }

    pushNavState();

    if (!m_activeFeaturedPlaylistId.isEmpty()) {
        m_activeFeaturedPlaylistId.clear();
        emit activeFeaturedPlaylistChanged();
    }
    setActivePlaylistId(id);
    setCurrentPage(3);
}

void AppController::openFeaturedPlaylist(const QString& id)
{
    const FeaturedPlaylist* playlist = FeaturedPlaylistCatalog::find(id);
    if (!playlist) {
        return;
    }

    if (m_activeFeaturedPlaylistId == id && m_currentPage == 4) {
        if (hasFeaturedCache(id, 1)) {
            applyFeaturedCacheToUi(id, 1);
        }
        return;
    }

    pushNavState();

    if (!m_activePlaylistId.isEmpty()) {
        setActivePlaylistId(QString());
    }

    m_activeFeaturedPlaylistId = id;
    emit activeFeaturedPlaylistChanged();

    setCurrentPage(4);

    if (hasFeaturedCache(id, 1)) {
        applyFeaturedCacheToUi(id, 1);
    } else {
        searchFeaturedPlaylist(playlist->keyword, 1);
    }
}

void AppController::toggleFavoriteFeaturedPlaylist()
{
    if (m_activeFeaturedPlaylistId.isEmpty()) {
        return;
    }

    if (m_favoriteFeaturedPlaylistIds.contains(m_activeFeaturedPlaylistId)) {
        m_favoriteFeaturedPlaylistIds.removeAll(m_activeFeaturedPlaylistId);
        setStatus(QStringLiteral("已取消收藏「%1」").arg(activeFeaturedPlaylistTitle()));
    } else {
        m_favoriteFeaturedPlaylistIds.append(m_activeFeaturedPlaylistId);
        setStatus(QStringLiteral("已收藏「%1」").arg(activeFeaturedPlaylistTitle()));
    }

    saveFavoriteFeaturedPlaylists();
    emit favoriteFeaturedPlaylistsChanged();
    emit activeFeaturedPlaylistChanged();
}

void AppController::playPlaylistRow(int row)
{
    if (row < 0 || row >= m_playlistTrackModel.rowCount()) {
        return;
    }

    m_isOnlinePlayback = true;
    m_onlineQueueType = OnlineQueueType::Playlist;
    m_onlineQueueRow = row;
    m_pendingSearchRow = -1;
    m_pendingPlaylistRow = row;
    if (!m_canControl) {
        m_canControl = true;
        emit canControlChanged();
    }
    m_playlistTrackModel.setPlayingRow(row);
    m_searchResultModel.setPlayingRow(-1);
    m_playlistTrackModel.setSelectedRow(row);

    const QVector<PlaylistTrackEntry>& entries = m_playlistTrackModel.entries();
    const PlaylistTrackRef& ref = entries.at(row).ref;
    m_currentAlbum = ref.album.isEmpty() ? QStringLiteral("未知专辑") : ref.album;
    m_currentSource = ref.localPath.isEmpty()
        ? registrySourceLabel(m_sourceRegistry, ref.sourceId, ref.songId)
        : QStringLiteral("本地音乐");
    m_currentSongId = ref.songId;
    m_currentLocalPath = ref.localPath;

    if (!ref.localPath.isEmpty() && QFileInfo::exists(ref.localPath)) {
        cancelPendingOnlineRequests();
        m_isOnlinePlayback = false;
        m_pendingSearchRow = -1;
        m_pendingPlaylistRow = -1;

        int localRow = -1;
        for (int i = 0; i < m_trackModel.rowCount(); ++i) {
            if (m_trackModel.filePathAt(i) == ref.localPath) {
                localRow = i;
                break;
            }
        }
        if (localRow >= 0) {
            playRow(localRow);
            m_onlineQueueType = OnlineQueueType::Playlist;
            m_onlineQueueRow = row;
            m_currentAlbum = ref.album.isEmpty() ? QStringLiteral("未知专辑") : ref.album;
            m_currentSource = QStringLiteral("本地音乐");
            m_currentSongId = ref.songId;
            m_currentLocalPath = ref.localPath;
            emit nowPlayingChanged();
            updateCurrentLikeState();
            return;
        }

        m_currentRow = -1;
        m_currentFilePath = ref.localPath;
        m_currentTitle = ref.title;
        m_currentArtist = ref.artist.isEmpty() ? QStringLiteral("未知艺术家") : ref.artist;
        m_currentCover = {};
        m_hasCover = false;
        m_currentAlbum = ref.album.isEmpty() ? QStringLiteral("未知专辑") : ref.album;
        m_currentSource = QStringLiteral("本地音乐");
        m_currentSongId = ref.songId;
        m_currentLocalPath = ref.localPath;
        if (!m_canControl) {
            m_canControl = true;
            emit canControlChanged();
        }
        m_audioPlayer.load(ref.localPath);
        m_currentSubtitle = QStringLiteral("播放中");
        emit nowPlayingChanged();
        updateCurrentLikeState();
        setStatus(QStringLiteral("播放中"));
        m_audioPlayer.play();
        return;
    }

    playOnlineTrackRef(ref, true);
}

void AppController::removePlaylistTrack(int row)
{
    if (row < 0 || row >= m_playlistTrackModel.rowCount()) {
        return;
    }
    const QString songId = m_playlistTrackModel.songIdAt(row);
    if (songId.isEmpty()) {
        return;
    }
    if (!m_playlistStore.removeTrack(m_activePlaylistId, songId)) {
        return;
    }
    reloadActivePlaylistModel();
    syncSearchLikedStates();
    syncLocalLikedStates();
    setStatus(QStringLiteral("已从歌单移除"));
}

void AppController::removeTrackFromActivePlaylist(int row)
{
    if (m_activePlaylistId == QStringLiteral("liked")) {
        toggleLikePlaylistRow(row);
        return;
    }
    removePlaylistTrack(row);
}

PlaylistTrackRef AppController::trackRefFromPlaylistRow(int row) const
{
    PlaylistTrackRef ref;
    if (row < 0 || row >= m_playlistTrackModel.rowCount()) {
        return ref;
    }
    ref = m_playlistTrackModel.entries().at(row).ref;
    if (ref.sourceId.isEmpty()) {
        QString src;
        if (OnlineSongId::parse(ref.songId, &src, nullptr)) {
            ref.sourceId = src;
        }
    }
    return ref;
}

bool AppController::isPlaylistRowLiked(int row) const
{
    const PlaylistTrackRef ref = trackRefFromPlaylistRow(row);
    if (ref.songId.isEmpty()) {
        return false;
    }
    return m_playlistStore.containsTrack(QStringLiteral("liked"), ref.songId);
}

void AppController::toggleLikePlaylistRow(int row)
{
    const PlaylistTrackRef ref = trackRefFromPlaylistRow(row);
    if (ref.songId.isEmpty()) {
        return;
    }

    const bool liked = m_playlistStore.containsTrack(QStringLiteral("liked"), ref.songId);
    if (liked) {
        m_playlistStore.removeTrack(QStringLiteral("liked"), ref.songId);
        syncSearchLikedStates();
        syncLocalLikedStates();
        if (m_activePlaylistId == QStringLiteral("liked")) {
            reloadActivePlaylistModel();
        } else {
            m_playlistTrackModel.refreshLikedState(row, false);
        }
        setStatus(QStringLiteral("已取消喜欢"));
        return;
    }

    if (m_playlistStore.addTrack(QStringLiteral("liked"), ref)) {
        syncSearchLikedStates();
        syncLocalLikedStates();
        m_playlistTrackModel.refreshLikedState(row, true);
        setStatus(QStringLiteral("已加入「我喜欢的音乐」"));
    }
}

void AppController::toggleCurrentLike()
{
    if (m_currentSongId.isEmpty()) {
        return;
    }

    PlaylistTrackRef ref;
    ref.songId = m_currentSongId;
    ref.title = m_currentTitle;
    ref.artist = m_currentArtist;
    ref.album = m_currentAlbum;
    ref.localPath = m_currentLocalPath;

    if (isCurrentTrackLiked()) {
        m_playlistStore.removeTrack(QStringLiteral("liked"), m_currentSongId);
        setStatus(QStringLiteral("已取消喜欢"));
    } else if (m_playlistStore.addTrack(QStringLiteral("liked"), ref)) {
        setStatus(QStringLiteral("已加入「我喜欢的音乐」"));
    } else {
        return;
    }

    syncSearchLikedStates();
    syncLocalLikedStates();
    syncPlaylistLikedStates();
    emit currentLikeChanged();
}
