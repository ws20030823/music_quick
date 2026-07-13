#pragma once

// =============================================================================
// AppController — QML 与 core/media/network 的唯一桥接（app 层）
// =============================================================================
// 职责：
//   - 本地播放：TrackListModel + AudioPlayer + PlaybackController
//   - 在线搜索：SearchResultModel + MyFreeMp3Client（MyFreeMp3 站 HTML 解析）
//   - 通过 Q_PROPERTY / Q_INVOKABLE 暴露给 QML（contextProperty 名 "app"）
// =============================================================================

#include "app/FeaturedPlaylistCatalog.h"
#include "app/FeaturedPlaylistCacheStore.h"
#include "app/MediaCacheManager.h"
#include "app/PlaylistImportManager.h"
#include "app/PlaylistImportPreviewModel.h"
#include "app/PlaylistStore.h"
#include "app/PlaylistTrackModel.h"
#include "app/SearchResultModel.h"
#include "app/TrackListModel.h"
#include "core/AudioPlayer.h"
#include "core/PlaybackController.h"
#include "core/PlaybackMode.h"
#include "network/MusicSourceRegistry.h"
#include "network/LocalStreamProxy.h"
#include "network/OnlineTrack.h"
#include "network/StreamFetchOptions.h"

#include <QColor>
#include <QHash>
#include <QImage>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QMediaPlayer>

class MusicSourceClient;
class OnlineStreamLoader;

class AppController final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(TrackListModel* trackModel READ trackModel CONSTANT)
    Q_PROPERTY(SearchResultModel* searchResultModel READ searchResultModel CONSTANT)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(bool canControl READ canControl NOTIFY canControlChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentSubtitle READ currentSubtitle NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentAlbum READ currentAlbum NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentSource READ currentSource NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentLyrics READ currentLyrics NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentSongId READ currentSongId NOTIFY nowPlayingChanged)
    Q_PROPERTY(bool isOnlinePlayback READ isOnlinePlayback NOTIFY nowPlayingChanged)
    Q_PROPERTY(bool isCurrentTrackLiked READ isCurrentTrackLiked NOTIFY currentLikeChanged)
    Q_PROPERTY(bool nowPlayingVisible READ nowPlayingVisible WRITE setNowPlayingVisible NOTIFY nowPlayingVisibleChanged)
    Q_PROPERTY(bool hasCover READ hasCover NOTIFY nowPlayingChanged)
    Q_PROPERTY(QString currentCoverUrl READ currentCoverUrl NOTIFY nowPlayingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(int playbackMode READ playbackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(QString playbackModeTooltip READ playbackModeTooltip NOTIFY playbackModeChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int trackCount READ trackCount NOTIFY trackCountChanged)
    Q_PROPERTY(bool queueVisible READ queueVisible WRITE setQueueVisible NOTIFY queueVisibleChanged)

    // --- 在线搜索状态（供 SearchPage 绑定）---
    Q_PROPERTY(bool searchBusy READ searchBusy NOTIFY searchBusyChanged)
    Q_PROPERTY(QString searchKeyword READ searchKeyword NOTIFY searchKeywordChanged)
    Q_PROPERTY(QString searchStatus READ searchStatus NOTIFY searchStatusChanged)
    Q_PROPERTY(int searchCurrentPage READ searchCurrentPage NOTIFY searchPaginationChanged)
    Q_PROPERTY(bool searchHasNext READ searchHasNext NOTIFY searchPaginationChanged)
    Q_PROPERTY(bool searchHasPrevious READ searchHasPrevious NOTIFY searchPaginationChanged)
    Q_PROPERTY(int searchResultCount READ searchResultCount NOTIFY searchResultCountChanged)

    Q_PROPERTY(PlaylistTrackModel* playlistTrackModel READ playlistTrackModel CONSTANT)
    Q_PROPERTY(PlaylistImportPreviewModel* playlistImportModel READ playlistImportModel CONSTANT)
    Q_PROPERTY(QVariantList sidebarPlaylists READ sidebarPlaylists NOTIFY playlistsChanged)
    Q_PROPERTY(QString activePlaylistId READ activePlaylistId WRITE setActivePlaylistId NOTIFY activePlaylistIdChanged)
    Q_PROPERTY(QString activePlaylistName READ activePlaylistName NOTIFY activePlaylistContentChanged)
    Q_PROPERTY(int activePlaylistTrackCount READ activePlaylistTrackCount NOTIFY activePlaylistContentChanged)
    Q_PROPERTY(int likedTrackCount READ likedTrackCount NOTIFY playlistsChanged)
    Q_PROPERTY(bool playlistImportBusy READ playlistImportBusy NOTIFY playlistImportBusyChanged)
    Q_PROPERTY(QString playlistImportStatus READ playlistImportStatus NOTIFY playlistImportStatusChanged)
    Q_PROPERTY(QString playlistImportPlatform READ playlistImportPlatform NOTIFY playlistImportPlatformChanged)
    Q_PROPERTY(int playlistImportMatchedCount READ playlistImportMatchedCount NOTIFY playlistImportCountsChanged)
    Q_PROPERTY(int playlistImportUnmatchedCount READ playlistImportUnmatchedCount NOTIFY playlistImportCountsChanged)
    Q_PROPERTY(int playlistImportProcessedCount READ playlistImportProcessedCount NOTIFY playlistImportCountsChanged)
    Q_PROPERTY(int playlistImportTotalCount READ playlistImportTotalCount NOTIFY playlistImportCountsChanged)

    Q_PROPERTY(QString activeMusicSourceId READ activeMusicSourceId WRITE setActiveMusicSourceId NOTIFY activeMusicSourceChanged)
    Q_PROPERTY(QString activeMusicSourceName READ activeMusicSourceName NOTIFY activeMusicSourceChanged)
    Q_PROPERTY(QVariantList musicSources READ musicSources CONSTANT)

    Q_PROPERTY(QVariantList featuredPlaylists READ featuredPlaylists CONSTANT)
    Q_PROPERTY(QVariantList favoriteFeaturedPlaylists READ favoriteFeaturedPlaylists NOTIFY favoriteFeaturedPlaylistsChanged)
    Q_PROPERTY(QString activeFeaturedPlaylistId READ activeFeaturedPlaylistId NOTIFY activeFeaturedPlaylistChanged)
    Q_PROPERTY(QString activeFeaturedPlaylistTitle READ activeFeaturedPlaylistTitle NOTIFY activeFeaturedPlaylistChanged)
    Q_PROPERTY(QString activeFeaturedPlaylistSubtitle READ activeFeaturedPlaylistSubtitle NOTIFY activeFeaturedPlaylistChanged)
    Q_PROPERTY(QString activeFeaturedPlaylistCoverUrl READ activeFeaturedPlaylistCoverUrl NOTIFY activeFeaturedPlaylistChanged)
    Q_PROPERTY(bool activeFeaturedPlaylistFavorited READ activeFeaturedPlaylistFavorited NOTIFY activeFeaturedPlaylistChanged)

    Q_PROPERTY(bool canNavigateBack READ canNavigateBack NOTIFY canNavigateBackChanged)

    Q_PROPERTY(QString cacheDirectory READ cacheDirectory WRITE setCacheDirectory NOTIFY cacheSettingsChanged)
    Q_PROPERTY(int cacheMaxSizeMb READ cacheMaxSizeMb WRITE setCacheMaxSizeMb NOTIFY cacheSettingsChanged)
    Q_PROPERTY(QString cacheUsedText READ cacheUsedText NOTIFY cacheUsageChanged)
    Q_PROPERTY(qreal uiBackgroundOpacity READ uiBackgroundOpacity WRITE setUiBackgroundOpacity NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(qreal uiCardShellOpacity READ uiCardShellOpacity WRITE setUiCardShellOpacity NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(bool hasHomeWallpaper READ hasHomeWallpaper NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(QString homeWallpaperPath READ homeWallpaperPath WRITE setHomeWallpaperPath NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(QUrl homeWallpaperUrl READ homeWallpaperUrl NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(bool uiCustomTextColorEnabled READ uiCustomTextColorEnabled WRITE setUiCustomTextColorEnabled NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(QColor uiTextColor READ uiTextColor WRITE setUiTextColor NOTIFY appearanceSettingsChanged)
    Q_PROPERTY(bool launchAtStartup READ launchAtStartup WRITE setLaunchAtStartup NOTIFY generalSettingsChanged)
    Q_PROPERTY(bool settingsVisible READ settingsVisible WRITE setSettingsVisible NOTIFY settingsVisibleChanged)
    Q_PROPERTY(int settingsSection READ settingsSection WRITE setSettingsSection NOTIFY settingsSectionChanged)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    TrackListModel* trackModel();
    SearchResultModel* searchResultModel();

    int currentPage() const;
    void setCurrentPage(int page);

    bool isPlaying() const;
    bool canControl() const;
    QString currentTitle() const;
    QString currentArtist() const;
    QString currentSubtitle() const;
    QString currentAlbum() const;
    QString currentSource() const;
    QString currentLyrics() const;
    QString currentSongId() const;
    bool isOnlinePlayback() const;
    bool isCurrentTrackLiked() const;
    bool nowPlayingVisible() const;
    void setNowPlayingVisible(bool visible);
    Q_INVOKABLE void openNowPlaying();
    Q_INVOKABLE void closeNowPlaying();
    bool hasCover() const;
    QString currentCoverUrl() const;
    QImage currentCover() const;
    qint64 position() const;
    qint64 duration() const;
    int volume() const;
    void setVolume(int value);
    int playbackMode() const;
    QString playbackModeTooltip() const;
    QString statusText() const;
    int trackCount() const;
    bool queueVisible() const;
    void setQueueVisible(bool visible);

    bool searchBusy() const;
    QString searchKeyword() const;
    QString searchStatus() const;
    int searchCurrentPage() const;
    bool searchHasNext() const;
    bool searchHasPrevious() const;
    int searchResultCount() const;

    PlaylistTrackModel* playlistTrackModel();
    PlaylistImportPreviewModel* playlistImportModel();
    QVariantList sidebarPlaylists() const;
    QString activePlaylistId() const;
    void setActivePlaylistId(const QString& id);
    QString activePlaylistName() const;
    int activePlaylistTrackCount() const;
    int likedTrackCount() const;
    bool playlistImportBusy() const;
    QString playlistImportStatus() const;
    QString playlistImportPlatform() const;
    int playlistImportMatchedCount() const;
    int playlistImportUnmatchedCount() const;
    int playlistImportProcessedCount() const;
    int playlistImportTotalCount() const;

    QVariantList featuredPlaylists() const;
    QVariantList favoriteFeaturedPlaylists() const;
    QString activeFeaturedPlaylistId() const;
    QString activeFeaturedPlaylistTitle() const;
    QString activeFeaturedPlaylistSubtitle() const;
    QString activeFeaturedPlaylistCoverUrl() const;
    bool activeFeaturedPlaylistFavorited() const;
    bool canNavigateBack() const;

    QString activeMusicSourceId() const;
    void setActiveMusicSourceId(const QString& id);
    QString activeMusicSourceName() const;
    QVariantList musicSources() const;

    Q_INVOKABLE void importFiles(const QList<QUrl>& urls);
    Q_INVOKABLE void togglePlayback();
    Q_INVOKABLE void playRow(int row);
    Q_INVOKABLE void selectLocalRow(int row);
    Q_INVOKABLE void selectSearchRow(int row);
    Q_INVOKABLE void selectPlaylistRow(int row);
    Q_INVOKABLE void toggleLikeLocalRow(int row);
    Q_INVOKABLE void playAllLocal();
    Q_INVOKABLE void playNext();
    Q_INVOKABLE void playPrevious();
    Q_INVOKABLE void cyclePlaybackMode();
    Q_INVOKABLE void seek(qint64 ms);
    Q_INVOKABLE QVariantList queueItems() const;
    Q_INVOKABLE void playQueueRow(int row);

    // --- 在线搜索：由 TopBar / SearchPage 调用 ---
    Q_INVOKABLE void searchOnline(const QString& keyword, int page = 1);
    Q_INVOKABLE void searchNextPage();
    Q_INVOKABLE void searchPreviousPage();
    Q_INVOKABLE void playSearchRow(int row);

    Q_INVOKABLE void playAllSearchResults();
    Q_INVOKABLE void likeAllSearchResults();
    Q_INVOKABLE void toggleLikeSearchRow(int row);
    Q_INVOKABLE bool isSearchRowLiked(int row) const;
    Q_INVOKABLE void addSearchRowToPlaylist(int row, const QString& playlistId);
    Q_INVOKABLE QString createPlaylist(const QString& name);
    Q_INVOKABLE bool deletePlaylist(const QString& id);
    Q_INVOKABLE void openPlaylist(const QString& id);
    Q_INVOKABLE void previewExternalPlaylist(const QString& url);
    Q_INVOKABLE QVariantMap confirmPlaylistImport(const QString& targetPlaylistId,
                                                  const QString& newPlaylistName = {});
    Q_INVOKABLE void cancelPlaylistImport();
    Q_INVOKABLE void clearPlaylistImportPreview();
    Q_INVOKABLE void openFeaturedPlaylist(const QString& id);
    Q_INVOKABLE void toggleFavoriteFeaturedPlaylist();
    Q_INVOKABLE bool isFeaturedPlaylistFavorited(const QString& id) const;

    Q_INVOKABLE void navigateToPage(int page);
    Q_INVOKABLE void navigateBack();

    QString cacheDirectory() const;
    void setCacheDirectory(const QString& path);
    int cacheMaxSizeMb() const;
    void setCacheMaxSizeMb(int megabytes);
    QString cacheUsedText() const;
    Q_INVOKABLE void refreshCacheUsage();
    Q_INVOKABLE void clearMediaCache();

    qreal uiBackgroundOpacity() const;
    void setUiBackgroundOpacity(qreal value);
    qreal uiCardShellOpacity() const;
    void setUiCardShellOpacity(qreal value);
    bool hasHomeWallpaper() const;
    QString homeWallpaperPath() const;
    void setHomeWallpaperPath(const QString& path);
    QUrl homeWallpaperUrl() const;
    bool uiCustomTextColorEnabled() const;
    void setUiCustomTextColorEnabled(bool value);
    QColor uiTextColor() const;
    void setUiTextColor(const QColor& value);
    bool launchAtStartup() const;
    void setLaunchAtStartup(bool enabled);

    bool settingsVisible() const;
    void setSettingsVisible(bool visible);
    int settingsSection() const;
    void setSettingsSection(int section);
    Q_INVOKABLE void toggleSettings();
    Q_INVOKABLE void openSettings(int section);
    Q_INVOKABLE void requestCloseSettings();
    Q_INVOKABLE void closeSettings();

    Q_INVOKABLE void refreshActivePlaylist();
    Q_INVOKABLE void playPlaylistRow(int row);
    Q_INVOKABLE void removePlaylistTrack(int row);
    Q_INVOKABLE void removeTrackFromActivePlaylist(int row);
    Q_INVOKABLE int removeTracksFromActivePlaylist(const QVariantList& rows);
    Q_INVOKABLE void toggleLikePlaylistRow(int row);
    Q_INVOKABLE bool isPlaylistRowLiked(int row) const;
    Q_INVOKABLE void toggleCurrentLike();

signals:
    void currentPageChanged();
    void playbackStateChanged();
    void nowPlayingChanged();
    void currentLikeChanged();
    void nowPlayingVisibleChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void playbackModeChanged();
    void statusTextChanged();
    void trackCountChanged();
    void queueVisibleChanged();
    void canControlChanged();

    void searchBusyChanged();
    void searchKeywordChanged();
    void searchStatusChanged();
    void searchPaginationChanged();
    void searchResultCountChanged();
    void playlistsChanged();
    void activePlaylistIdChanged();
    void activePlaylistContentChanged();
    void playlistImportBusyChanged();
    void playlistImportStatusChanged();
    void playlistImportPlatformChanged();
    void playlistImportCountsChanged();
    void activeMusicSourceChanged();
    void favoriteFeaturedPlaylistsChanged();
    void activeFeaturedPlaylistChanged();
    void canNavigateBackChanged();
    void cacheSettingsChanged();
    void cacheUsageChanged();
    void appearanceSettingsChanged();
    void generalSettingsChanged();
    void settingsVisibleChanged();
    void settingsSectionChanged();
    void settingsCloseRequested();

private slots:
    void onSearchCompleted(const SearchPageResult& result, const QString& keyword);
    void onSearchFailed(const QString& message);
    void onStreamUrlResolved(const OnlineTrack& track);
    void onStreamUrlFailed(const QString& songId, const QString& message);
    void onStreamPrefetchReady(const QString& localFilePath, const QUrl& originalUrl);
    void onStreamPrefetchFailed(const QUrl& originalUrl, const QString& message);

private:
    void loadTrack(int row, bool autoPlay);
    void loadOnlineStream(const QString& streamUrl,
                          const QString& title,
                          const QString& artist,
                          const QImage& cover,
                          bool autoPlay,
                          const QString& lyrics = {});
    void rebuildShuffleOrder();
    void applyNavigateResult(const PlaybackNavigateResult& result, int currentRow);
    void updatePlaybackModeProperties();
    void setStatus(const QString& text);
    void setSearchBusy(bool busy);
    void setSearchStatus(const QString& text);
    void applySearchPageResult(const SearchPageResult& result);
    void downloadNowPlayingCover(const QUrl& coverUrl, const QString& songId = {});
    void beginOnlinePlaybackUi(const QString& subtitle);
    void queueSearchStreamPrefetch();
    void queueAdjacentOnlineStreamPrefetch();
    void startNextStreamPrefetch();
    void cancelStreamUrlPrefetch();
    void prefetchResolvedOnlineStream(const QString& streamUrl, const QString& sourceId);
    void syncSearchLikedStates();
    void syncLocalLikedStates();
    void syncPlaylistLikedStates();
    void searchFeaturedPlaylist(const QString& keyword, int page);
    void requestFeaturedSearch(const QString& playlistId,
                               const QString& keyword,
                               int page,
                               bool applyToUi);
    void prefetchFeaturedPlaylists();
    void startNextFeaturedPrefetch();
    bool hasFeaturedCache(const QString& playlistId, int page) const;
    const SearchPageResult* featuredCache(const QString& playlistId, int page) const;
    void storeFeaturedCache(const QString& playlistId, int page, const SearchPageResult& result);
    void applyFeaturedCacheToUi(const QString& playlistId, int page);
    QString featuredSearchStatusText(const SearchPageResult& result) const;
    QString activeFeaturedKeyword() const;
    void pushNavState();
    void loadFavoriteFeaturedPlaylists();
    void saveFavoriteFeaturedPlaylists() const;
    void loadAppearanceSettings();
    void saveAppearanceSettings() const;
    void loadGeneralSettings();
    void saveGeneralSettings() const;

    void reloadActivePlaylistModel();
    PlaylistTrackRef trackRefFromSearchRow(int row) const;
    PlaylistTrackRef trackRefFromLocalRow(int row) const;
    PlaylistTrackRef trackRefFromPlaylistRow(int row) const;
    void playOnlineTrackRef(const PlaylistTrackRef& track, bool autoPlay);
    void playOnlineNext();
    void playOnlinePrevious();
    void handleOnlinePlaybackFailure(const QString& message);
    void cancelPendingOnlineRequests();
    void connectAllSourceClients();
    void updateCurrentLikeState();
    bool isPendingOnlineSearchRow(int row) const;
    bool isPendingOnlinePlaylistRow(int row) const;

    enum class OnlineQueueType { None, Search, Playlist };

    struct StreamPrefetchRequest {
        OnlineQueueType type = OnlineQueueType::None;
        int row = -1;
    };

    struct FeaturedSearchRequest {
        QString playlistId;
        QString keyword;
        int page = 1;
        bool applyToUi = false;
    };

    struct NavSnapshot {
        int page = 0;
        QString featuredPlaylistId;
        QString playlistId;
    };

    TrackListModel m_trackModel;
    SearchResultModel m_searchResultModel;
    PlaylistTrackModel m_playlistTrackModel;
    PlaylistStore m_playlistStore;
    PlaylistImportManager* m_playlistImportManager = nullptr;
    MediaCacheManager m_mediaCache;
    AudioPlayer m_audioPlayer;
    PlaybackController m_playbackController;
    MusicSourceRegistry m_sourceRegistry;
    OnlineStreamLoader* m_streamLoader = nullptr;
    OnlineStreamLoader* m_backgroundStreamLoader = nullptr;
    LocalStreamProxy* m_streamProxy = nullptr;
    QNetworkAccessManager m_coverNetwork;

    bool m_pendingStreamAutoPlay = false;
    QString m_pendingStreamUrl;
    StreamFetchOptions m_currentStreamFetchOptions;

    int m_currentPage = 0;
    int m_currentRow = -1;
    bool m_isOnlinePlayback = false;
    int m_pendingSearchRow = -1;

    QString m_currentFilePath;
    QString m_currentTitle;
    QString m_currentArtist;
    QString m_currentSubtitle;
    QString m_currentAlbum;
    QString m_currentSource;
    QString m_currentLyrics;
    QString m_currentSongId;
    QString m_currentLocalPath;
    QImage m_currentCover;
    bool m_nowPlayingVisible = false;
    bool m_hasCover = false;
    bool m_canControl = false;
    int m_volume = 50;
    int m_playbackMode = static_cast<int>(PlaybackDisplayMode::Sequential);
    QString m_playbackModeTooltip = QStringLiteral("顺序播放");
    QString m_statusText = QStringLiteral("就绪");
    bool m_queueVisible = false;

    QString m_searchKeyword;
    QString m_searchStatus = QStringLiteral("输入关键词，探索在线曲库");
    bool m_searchBusy = false;
    int m_searchCurrentPage = 1;
    bool m_searchHasNext = false;
    bool m_searchHasPrevious = false;

    QString m_activePlaylistId;
    QString m_activeFeaturedPlaylistId;
    QStringList m_favoriteFeaturedPlaylistIds;

    QHash<QString, QHash<int, SearchPageResult>> m_featuredPlaylistCache;
    QStringList m_featuredPrefetchQueue;
    FeaturedSearchRequest m_inFlightFeaturedSearch;
    bool m_featuredSearchInFlight = false;
    bool m_hasPendingFeaturedUiRequest = false;
    FeaturedSearchRequest m_pendingFeaturedUiRequest;

    QVector<NavSnapshot> m_navBackStack;
    bool m_navigatingBack = false;

    int m_pendingPlaylistRow = -1;
    QString m_activeMusicSourceId = QStringLiteral("gequbao");
    QVariantList m_musicSources;

    OnlineQueueType m_onlineQueueType = OnlineQueueType::None;
    int m_onlineQueueRow = -1;

    QList<StreamPrefetchRequest> m_streamPrefetchQueue;
    bool m_streamPrefetchInFlight = false;
    OnlineQueueType m_activeStreamPrefetchType = OnlineQueueType::None;
    int m_activeStreamPrefetchRow = -1;
    static constexpr int kStreamPrefetchCount = 3;

    qreal m_uiBackgroundOpacity = 1.0;
    qreal m_uiCardShellOpacity = 0.20;
    QString m_homeWallpaperPath;
    bool m_uiCustomTextColorEnabled = false;
    QColor m_uiTextColor = QColor(QStringLiteral("#1B1B1F"));
    bool m_launchAtStartup = false;
    bool m_settingsVisible = false;
    int m_settingsSection = 0;
};
