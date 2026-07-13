#pragma once

#include <QByteArray>
#include <QList>
#include <QPair>
#include <QString>
#include <QUrl>
#include <QVector>

enum class ExternalPlaylistPlatform {
    Unknown,
    Netease,
    QQMusic,
};

struct ExternalPlaylistRef {
    ExternalPlaylistPlatform platform = ExternalPlaylistPlatform::Unknown;
    QString id;

    bool isValid() const { return platform != ExternalPlaylistPlatform::Unknown && !id.isEmpty(); }
    QString platformName() const;
};

struct ExternalPlaylistTrack {
    QString title;
    QString artist;
};

struct ExternalPlaylistParseResult {
    QVector<ExternalPlaylistTrack> tracks;
    QVector<qint64> trackIds;
    QString error;
};

using ExternalPlaylistHeaders = QList<QPair<QByteArray, QByteArray>>;

ExternalPlaylistRef parseExternalPlaylistUrl(const QString& raw);
ExternalPlaylistParseResult parseNeteasePlaylistJson(const QByteArray& body);
ExternalPlaylistParseResult parseNeteaseSongDetailJson(const QByteArray& body);
ExternalPlaylistParseResult parseQQPlaylistJson(const QByteArray& body);

QUrl buildNeteasePlaylistUrl(const QString& playlistId);
QUrl buildNeteaseSongDetailUrl(const QVector<qint64>& ids);
QUrl buildQQPlaylistUrl(const QString& disstid);
ExternalPlaylistHeaders neteasePlaylistHeaders();
ExternalPlaylistHeaders qqPlaylistHeaders();
