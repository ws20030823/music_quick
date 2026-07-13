#include "network/ExternalPlaylistImporter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUrlQuery>

namespace {

constexpr auto kUserAgent =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36";

QPair<QByteArray, QByteArray> header(const char* name, const char* value)
{
    return qMakePair(QByteArray(name), QByteArray(value));
}

QString firstArtistFromNeteaseTrack(const QJsonObject& track)
{
    const QJsonArray ar = track.value(QStringLiteral("ar")).toArray();
    if (!ar.isEmpty()) {
        return ar.first().toObject().value(QStringLiteral("name")).toString();
    }

    const QJsonArray artists = track.value(QStringLiteral("artists")).toArray();
    if (!artists.isEmpty()) {
        return artists.first().toObject().value(QStringLiteral("name")).toString();
    }

    return QStringLiteral("—");
}

ExternalPlaylistParseResult parseNeteaseTrackArray(const QJsonArray& tracks)
{
    ExternalPlaylistParseResult result;
    result.tracks.reserve(tracks.size());
    for (const QJsonValue& value : tracks) {
        const QJsonObject obj = value.toObject();
        const QString title = obj.value(QStringLiteral("name")).toString().trimmed();
        if (title.isEmpty()) {
            continue;
        }
        ExternalPlaylistTrack track;
        track.title = title;
        track.artist = firstArtistFromNeteaseTrack(obj);
        result.tracks.append(track);
    }
    return result;
}

QJsonObject parseObject(const QByteArray& body, QString* error)
{
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error) {
            *error = QStringLiteral("返回数据不是有效 JSON");
        }
        return {};
    }
    return doc.object();
}

} // namespace

QString ExternalPlaylistRef::platformName() const
{
    switch (platform) {
    case ExternalPlaylistPlatform::Netease:
        return QStringLiteral("网易云音乐");
    case ExternalPlaylistPlatform::QQMusic:
        return QStringLiteral("QQ 音乐");
    case ExternalPlaylistPlatform::Unknown:
        break;
    }
    return QStringLiteral("未知平台");
}

ExternalPlaylistRef parseExternalPlaylistUrl(const QString& raw)
{
    const QString trimmed = raw.trimmed();
    if (trimmed.isEmpty() || trimmed.contains(QLatin1Char(' '))) {
        return {};
    }

    const QString normalized = trimmed.startsWith(QStringLiteral("http"), Qt::CaseInsensitive)
        ? trimmed
        : QStringLiteral("https://") + trimmed;
    const QUrl url(normalized);
    if (!url.isValid() || url.host().isEmpty()) {
        return {};
    }

    const QString host = url.host().toLower();
    const QUrlQuery query(url);

    if (host.contains(QStringLiteral("music.163.com"))) {
        const QString id = query.queryItemValue(QStringLiteral("id"));
        if (!id.isEmpty() && id.toLongLong() > 0) {
            return {ExternalPlaylistPlatform::Netease, id};
        }

        const QRegularExpression playlistRe(
            QStringLiteral(R"(music\.163\.com/(?:#/)?playlist\?id=(\d+))"));
        const QRegularExpressionMatch playlistMatch = playlistRe.match(trimmed);
        if (playlistMatch.hasMatch()) {
            return {ExternalPlaylistPlatform::Netease, playlistMatch.captured(1)};
        }

        const QRegularExpression idRe(QStringLiteral(R"(music\.163\.com/[^?\s]*[?&]id=(\d+))"));
        const QRegularExpressionMatch idMatch = idRe.match(trimmed);
        if (idMatch.hasMatch()) {
            return {ExternalPlaylistPlatform::Netease, idMatch.captured(1)};
        }
        return {};
    }

    if (host.contains(QStringLiteral("y.qq.com")) || host == QStringLiteral("i.y.qq.com")) {
        const QRegularExpression ryqqRe(QStringLiteral(R"(/n/ryqq/playlist/(\d+))"));
        const QRegularExpressionMatch ryqqMatch = ryqqRe.match(url.path());
        if (ryqqMatch.hasMatch()) {
            return {ExternalPlaylistPlatform::QQMusic, ryqqMatch.captured(1)};
        }

        const QString id = !query.queryItemValue(QStringLiteral("id")).isEmpty()
            ? query.queryItemValue(QStringLiteral("id"))
            : (!query.queryItemValue(QStringLiteral("disstid")).isEmpty()
                   ? query.queryItemValue(QStringLiteral("disstid"))
                   : query.queryItemValue(QStringLiteral("dissid")));
        if (!id.isEmpty() && id.toLongLong() > 0) {
            return {ExternalPlaylistPlatform::QQMusic, id};
        }

        const QRegularExpression pathRe(QStringLiteral(R"(/playlist/(\d+))"));
        const QRegularExpressionMatch pathMatch = pathRe.match(url.path());
        if (pathMatch.hasMatch()) {
            return {ExternalPlaylistPlatform::QQMusic, pathMatch.captured(1)};
        }
    }

    return {};
}

ExternalPlaylistParseResult parseNeteasePlaylistJson(const QByteArray& body)
{
    ExternalPlaylistParseResult result;
    QString error;
    const QJsonObject root = parseObject(body, &error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const int code = root.value(QStringLiteral("code")).toInt(200);
    if (code != 200) {
        result.error = QStringLiteral("网易云接口返回 code=%1，请确认歌单是否公开").arg(code);
        return result;
    }

    const QJsonObject bag = root.contains(QStringLiteral("playlist"))
        ? root.value(QStringLiteral("playlist")).toObject()
        : root.value(QStringLiteral("result")).toObject();
    if (bag.isEmpty()) {
        result.error = QStringLiteral("网易云返回数据格式异常");
        return result;
    }

    const QJsonArray trackIds = bag.value(QStringLiteral("trackIds")).toArray();
    const QJsonArray tracks = bag.value(QStringLiteral("tracks")).toArray();
    if (!trackIds.isEmpty() && trackIds.size() > tracks.size()) {
        result.trackIds.reserve(trackIds.size());
        for (const QJsonValue& value : trackIds) {
            const qint64 id = static_cast<qint64>(value.toObject().value(QStringLiteral("id")).toDouble());
            if (id > 0) {
                result.trackIds.append(id);
            }
        }
        if (!result.trackIds.isEmpty()) {
            return result;
        }
    }

    if (!tracks.isEmpty()) {
        return parseNeteaseTrackArray(tracks);
    }

    result.trackIds.reserve(trackIds.size());
    for (const QJsonValue& value : trackIds) {
        const qint64 id = static_cast<qint64>(value.toObject().value(QStringLiteral("id")).toDouble());
        if (id > 0) {
            result.trackIds.append(id);
        }
    }
    if (result.trackIds.isEmpty()) {
        result.error = QStringLiteral("网易云歌单为空或接口未返回曲目");
    }
    return result;
}

ExternalPlaylistParseResult parseNeteaseSongDetailJson(const QByteArray& body)
{
    ExternalPlaylistParseResult result;
    QString error;
    const QJsonObject root = parseObject(body, &error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }
    return parseNeteaseTrackArray(root.value(QStringLiteral("songs")).toArray());
}

ExternalPlaylistParseResult parseQQPlaylistJson(const QByteArray& body)
{
    ExternalPlaylistParseResult result;
    QString error;
    const QJsonObject root = parseObject(body, &error);
    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    const int code = root.value(QStringLiteral("code")).toInt(0);
    if (code != 0) {
        result.error = QStringLiteral("QQ 音乐接口返回 code=%1，请确认歌单是否公开").arg(code);
        return result;
    }

    const QJsonArray cdlist = root.value(QStringLiteral("cdlist")).toArray();
    const QJsonArray songlist = cdlist.isEmpty()
        ? QJsonArray()
        : cdlist.first().toObject().value(QStringLiteral("songlist")).toArray();
    if (songlist.isEmpty()) {
        result.error = QStringLiteral("QQ 音乐歌单为空或无法拉取");
        return result;
    }

    result.tracks.reserve(songlist.size());
    for (const QJsonValue& value : songlist) {
        const QJsonObject song = value.toObject();
        const QString title = song.value(QStringLiteral("songname")).toString().trimmed();
        if (title.isEmpty()) {
            continue;
        }

        QStringList singers;
        const QJsonArray singerArray = song.value(QStringLiteral("singer")).toArray();
        for (const QJsonValue& singerValue : singerArray) {
            const QString name = singerValue.toObject().value(QStringLiteral("name")).toString().trimmed();
            if (!name.isEmpty()) {
                singers.append(name);
            }
        }

        ExternalPlaylistTrack track;
        track.title = title;
        track.artist = singers.isEmpty() ? QStringLiteral("—") : singers.join(QStringLiteral(" / "));
        result.tracks.append(track);
    }
    return result;
}

QUrl buildNeteasePlaylistUrl(const QString& playlistId)
{
    QUrl url(QStringLiteral("https://music.163.com/api/v6/playlist/detail"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), playlistId);
    query.addQueryItem(QStringLiteral("n"), QStringLiteral("100000"));
    query.addQueryItem(QStringLiteral("s"), QStringLiteral("0"));
    url.setQuery(query);
    return url;
}

QUrl buildNeteaseSongDetailUrl(const QVector<qint64>& ids)
{
    QStringList parts;
    parts.reserve(ids.size());
    for (const qint64 id : ids) {
        parts.append(QString::number(id));
    }

    QUrl url(QStringLiteral("https://music.163.com/api/song/detail"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("ids"), QStringLiteral("[%1]").arg(parts.join(QLatin1Char(','))));
    url.setQuery(query);
    return url;
}

QUrl buildQQPlaylistUrl(const QString& disstid)
{
    QUrl url(QStringLiteral("https://i.y.qq.com/qzone-music/fcg-bin/fcg_ucc_getcdinfo_byids_cp.fcg"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("type"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("json"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("utf8"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("onlysong"), QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("nosign"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("disstid"), disstid);
    query.addQueryItem(QStringLiteral("g_tk"), QStringLiteral("5381"));
    query.addQueryItem(QStringLiteral("loginUin"), QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("hostUin"), QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("inCharset"), QStringLiteral("GB2312"));
    query.addQueryItem(QStringLiteral("outCharset"), QStringLiteral("utf-8"));
    query.addQueryItem(QStringLiteral("notice"), QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("platform"), QStringLiteral("yqq"));
    query.addQueryItem(QStringLiteral("needNewCode"), QStringLiteral("0"));
    url.setQuery(query);
    return url;
}

ExternalPlaylistHeaders neteasePlaylistHeaders()
{
    return {
        header("User-Agent", kUserAgent),
        header("Referer", "https://music.163.com/"),
        header("Accept", "application/json, text/plain, */*"),
    };
}

ExternalPlaylistHeaders qqPlaylistHeaders()
{
    return {
        header("User-Agent", kUserAgent),
        header("Referer", "https://y.qq.com/"),
        header("Accept", "application/json, text/plain, */*"),
    };
}
