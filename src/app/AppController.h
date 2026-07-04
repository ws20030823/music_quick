#pragma once

// =============================================================================
// AppController — QML 与 core/media/network 的唯一桥接（app 层）
// =============================================================================
// 职责：
//   - 本地播放：TrackListModel + AudioPlayer + PlaybackController
//   - 在线搜索：SearchResultModel + MyFreeMp3Client（MyFreeMp3 站 HTML 解析）
//   - 通过 Q_PROPERTY / Q_INVOKABLE 暴露给 QML（contextProperty 名 "app"）
// =============================================================================

#include "app/PlaylistStore.h"
#include "app/PlaylistTrackModel.h"
#include "app/SearchResultModel.h"
#include "app/TrackListModel.h"
#include "core/AudioPlayer.h"
#include "core/PlaybackController.h"
#include "core/PlaybackMode.h"
#include "network/MusicSourceRegistry.h"
#include "network/OnlineTrack.h"
#include "network/StreamFetchOptions.h"

#include <QImage>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVariantList>
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
    Q_PROPERTY(QVariantList sidebarPlaylists READ sidebarPlaylists NOTIFY playlistsChanged)
    Q_PROPERTY(QString activePlaylistId READ activePlaylistId WRITE setActivePlaylistId NOTIFY activePlaylistIdChanged)
    Q_PROPERTY(QString activePlaylistName READ activePlaylistName NOTIFY activePlaylistContentChanged)
    Q_PROPERTY(int activePlaylistTrackCount READ activePlaylistTrackCount NOTIFY activePlaylistContentChanged)
    Q_PROPERTY(int likedTrackCount READ likedTrackCount NOTIFY playlistsChanged)

    Q_PROPERTY(QString activeMusicSourceId READ activeMusicSourceId WRITE setActiveMusicSourceId NOTIFY activeMusicSourceChanged)
    Q_PROPERTY(QString activeMusicSourceName READ activeMusicSourceName NOTIFY activeMusicSourceChanged)
    Q_PROPERTY(QVariantList musicSources READ musicSources CONSTANT)

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
    QString currentSongId() const;
    bool isOnlinePlayback() const;
    bool isCurrentTrackLiked() const;
    bool nowPlayingVisible() const;
    void setNowPlayingVisible(bool visible);
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
    QVariantList sidebarPlaylists() const;
    QString activePlaylistId() const;
    void setActivePlaylistId(const QString& id);
    QString activePlaylistName() const;
    int activePlaylistTrackCount() const;
    int likedTrackCount() const;

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
    Q_INVOKABLE void refreshActivePlaylist();
    Q_INVOKABLE void playPlaylistRow(int row);
    Q_INVOKABLE void removePlaylistTrack(int row);
    Q_INVOKABLE void removeTrackFromActivePlaylist(int row);
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
    void activeMusicSourceChanged();

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
                          bool autoPlay);
    void rebuildShuffleOrder();
    void applyNavigateResult(const PlaybackNavigateResult& result, int currentRow);
    void updatePlaybackModeProperties();
    void setStatus(const QString& text);
    void setSearchBusy(bool busy);
    void setSearchStatus(const QString& text);
    void applySearchPageResult(const SearchPageResult& result);
    void downloadSearchCover(int row, const QUrl& coverUrl);
    void syncSearchLikedStates();
    void syncLocalLikedStates();
    void syncPlaylistLikedStates();
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

    TrackListModel m_trackModel;
    SearchResultModel m_searchResultModel;
    PlaylistTrackModel m_playlistTrackModel;
    PlaylistStore m_playlistStore;
    AudioPlayer m_audioPlayer;
    PlaybackController m_playbackController;
    MusicSourceRegistry m_sourceRegistry;
    OnlineStreamLoader* m_streamLoader = nullptr;
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
    int m_pendingPlaylistRow = -1;
    QString m_activeMusicSourceId = QStringLiteral("myfreemp3");
    QVariantList m_musicSources;

    OnlineQueueType m_onlineQueueType = OnlineQueueType::None;
    int m_onlineQueueRow = -1;
};
