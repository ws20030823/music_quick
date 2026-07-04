#include "network/GequbaoParser.h"

#include "network/OnlineSongId.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>

namespace {

constexpr auto kSourceId = "gequbao";
constexpr auto kBaseUrl = "https://www.gequbao.net";

QString decodeUrlComponent(const QString& encoded)
{
    return QUrl::fromPercentEncoding(encoded.toUtf8());
}

QString extractJsonStringField(const QString& json, const QString& key)
{
    const QRegularExpression fieldRe(
        QStringLiteral("\"%1\"\\s*:\\s*\"([^\"]*)\"").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = fieldRe.match(json);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return {};
}

QString extractJsonNumberField(const QString& json, const QString& key)
{
    const QRegularExpression fieldRe(
        QStringLiteral("\"%1\"\\s*:\\s*(\\d+)").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = fieldRe.match(json);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return {};
}

QString extractWindowString(const QString& html, const QString& key)
{
    const QRegularExpression windowRe(
        QStringLiteral(R"(window\.%1\s*=\s*'([^']*)')").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = windowRe.match(html);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return {};
}

void appendSearchTrack(SearchPageResult& result,
                       const QString& trackId,
                       const QString& title,
                       const QString& artist,
                       const QString& detailPath)
{
    OnlineTrack track;
    track.sourceId = QString::fromLatin1(kSourceId);
    track.songId = OnlineSongId::compose(track.sourceId, trackId);
    track.title = title.trimmed();
    track.artist = artist.trimmed();
    if (!track.title.isEmpty() && !track.artist.isEmpty()) {
        track.displayTitle = track.title + QStringLiteral(" - ") + track.artist;
    } else {
        track.displayTitle = track.title.isEmpty() ? track.artist : track.title;
    }
    track.detailUrl = QString::fromLatin1(kBaseUrl) + detailPath;
    result.tracks.append(track);
}

} // namespace

SearchPageResult parseGequbaoSearchHtml(const QString& html)
{
    SearchPageResult result;
    QSet<QString> seenSongIds;

    static const QRegularExpression searchMusicRe(
        QStringLiteral(
            R"re(href="(/search_music\?[^"]+)")re"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator searchIt = searchMusicRe.globalMatch(html);
    while (searchIt.hasNext()) {
        const QRegularExpressionMatch match = searchIt.next();
        QString detailPath = match.captured(1);
        detailPath.replace(QStringLiteral("&amp;"), QStringLiteral("&"));

        const QUrl detailUrl(QStringLiteral("https://local") + detailPath);
        const QUrlQuery query(detailUrl.query(QUrl::ComponentFormattingOption::FullyDecoded));
        const QString songId = query.queryItemValue(QStringLiteral("song_id"));
        if (songId.isEmpty() || seenSongIds.contains(songId)) {
            continue;
        }
        seenSongIds.insert(songId);

        const QString title = decodeUrlComponent(query.queryItemValue(QStringLiteral("title")));
        const QString artist = decodeUrlComponent(query.queryItemValue(QStringLiteral("singer")));
        appendSearchTrack(result, songId, title, artist, detailPath);
    }

    static const QRegularExpression musicLinkRe(
        QStringLiteral(
            R"re(href="/music/(\d+)"[^>]*title="([^"]*)"[^>]*class="[^"]*text-primary)re"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator musicIt = musicLinkRe.globalMatch(html);
    while (musicIt.hasNext()) {
        const QRegularExpressionMatch match = musicIt.next();
        const QString musicId = match.captured(1);
        if (seenSongIds.contains(musicId)) {
            continue;
        }
        seenSongIds.insert(musicId);

        const QString title = match.captured(2).trimmed();
        appendSearchTrack(result,
                          musicId,
                          title,
                          {},
                          QStringLiteral("/music/") + musicId);
    }

    static const QRegularExpression pageInfoRe(QStringLiteral(R"(>(\d+)/(\d+)</a></li>)"));
    const QRegularExpressionMatch pageMatch = pageInfoRe.match(html);
    if (pageMatch.hasMatch()) {
        result.currentPage = pageMatch.captured(1).toInt();
        result.totalPages = qMax(pageMatch.captured(2).toInt(), result.currentPage);
    }

    result.hasPrevious = result.currentPage > 1;
    result.hasNext = html.contains(QStringLiteral("rel=\"next\""));

    return result;
}

GequbaoMusicDetail parseGequbaoMusicDetailHtml(const QString& html)
{
    GequbaoMusicDetail detail;

    static const QRegularExpression appDataRe(
        QStringLiteral(R"(window\.appData\s*=\s*(\{[^;]+\});)"));
    const QRegularExpressionMatch appDataMatch = appDataRe.match(html);
    if (appDataMatch.hasMatch()) {
        const QString appDataJson = appDataMatch.captured(1);
        detail.mp3Id = extractJsonNumberField(appDataJson, QStringLiteral("mp3_id"));
        detail.playId = extractJsonStringField(appDataJson, QStringLiteral("play_id"));
        detail.title = extractJsonStringField(appDataJson, QStringLiteral("mp3_title"));
        detail.artist = extractJsonStringField(appDataJson, QStringLiteral("mp3_author"));
    }

    if (detail.playId.isEmpty()) {
        detail.playId = extractWindowString(html, QStringLiteral("play_id"));
    }
    if (detail.mp3Id.isEmpty()) {
        detail.mp3Id = extractWindowString(html, QStringLiteral("mp3_id"));
    }
    if (detail.title.isEmpty()) {
        detail.title = extractWindowString(html, QStringLiteral("mp3_title"));
    }
    if (detail.artist.isEmpty()) {
        detail.artist = extractWindowString(html, QStringLiteral("mp3_author"));
    }
    detail.coverUrl = extractWindowString(html, QStringLiteral("mp3_cover"));

    return detail;
}

QString parseGequbaoPlayUrlJson(const QString& json)
{
    const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8());
    if (!document.isObject()) {
        return {};
    }

    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("code")).toInt() != 1) {
        return {};
    }

    const QJsonObject data = root.value(QStringLiteral("data")).toObject();
    return data.value(QStringLiteral("url")).toString().trimmed();
}
