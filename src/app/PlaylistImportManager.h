#pragma once

#include "app/PlaylistImportPreviewModel.h"
#include "network/ExternalPlaylistImporter.h"

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QVector>

class MusicSourceClient;
class QNetworkRequest;
class QNetworkReply;
struct OnlineTrack;

class PlaylistImportManager final : public QObject
{
    Q_OBJECT

public:
    explicit PlaylistImportManager(QObject* parent = nullptr);

    PlaylistImportPreviewModel* model();
    bool busy() const;
    QString status() const;
    QString platformName() const;
    int matchedCount() const;
    int unmatchedCount() const;
    int processedCount() const;
    int totalCount() const;

    void preview(const QString& rawUrl);
    void cancel();
    void clear();

signals:
    void busyChanged();
    void statusChanged();
    void countsChanged();
    void platformChanged();
    void previewCompleted();
    void previewFailed(const QString& message);

private:
    void setBusy(bool busy);
    void setStatus(const QString& status);
    void setPlatformName(const QString& name);
    void resetRunState();
    void fetchExternalPlaylist(const ExternalPlaylistRef& ref);
    void fetchNeteaseSongDetailChunk();
    void handleExternalReply(QNetworkReply* reply,
                             const ExternalPlaylistRef& ref,
                             bool songDetailReply);
    QNetworkRequest buildRequest(const QUrl& url, const ExternalPlaylistHeaders& headers) const;

    void startMatching();
    void matchCurrentTrack();
    void tryNextSource();
    void appendMissingRow();
    void appendMatchedRow(const OnlineTrack& track, const QString& sourceLabel);

    PlaylistImportPreviewModel m_model;
    QNetworkAccessManager m_network;
    QVector<MusicSourceClient*> m_sources;
    QPointer<QNetworkReply> m_activeReply;

    bool m_busy = false;
    bool m_cancelled = false;
    QString m_status = QStringLiteral("贴上网易云或 QQ 音乐歌单链接开始导入");
    QString m_platformName;

    QVector<ExternalPlaylistTrack> m_tracks;
    QVector<qint64> m_pendingNeteaseIds;
    QVector<ExternalPlaylistTrack> m_collectedNeteaseDetails;
    int m_detailOffset = 0;
    int m_trackIndex = 0;
    int m_sourceIndex = 0;
};
