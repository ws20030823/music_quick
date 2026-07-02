#include "network/MyFreeMp3Parser.h"

#include <QRegularExpression>

namespace {

void splitArtistTitle(OnlineTrack& track)
{
    const int dash = track.displayTitle.indexOf(QLatin1Char('-'));
    if (dash > 0) {
        track.artist = track.displayTitle.left(dash).trimmed();
        track.title = track.displayTitle.mid(dash + 1).trimmed();
    } else {
        track.title = track.displayTitle;
    }
}

} // namespace

QString stripHtmlTags(const QString& htmlFragment)
{
    QString cleaned = htmlFragment;
    static const QRegularExpression tagRe(QStringLiteral("<[^>]+>"));
    return cleaned.remove(tagRe).trimmed();
}

SearchPageResult parseSearchResultsHtml(const QString& html)
{
    SearchPageResult result;

    // 使用 R"re(...)re" 避免模式中的 )" 提前结束 raw string
    static const QRegularExpression trackRe(
        QStringLiteral(
            R"re(href="https?://myfreemp3online\.com/song/(\d+)\.html"\s+class="text-primary font-weight-bold"[^>]*title="([^"]*)")re"),
        QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator it = trackRe.globalMatch(html);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        OnlineTrack track;
        track.songId = match.captured(1);
        const QString rawTitle = match.captured(2).section(QStringLiteral(" MP3"), 0, 0).trimmed();
        track.displayTitle = stripHtmlTags(rawTitle);
        track.detailUrl = QStringLiteral("https://myfreemp3online.com/song/%1.html").arg(track.songId);
        splitArtistTitle(track);
        result.tracks.append(track);
    }

    static const QRegularExpression activePageRe(
        QStringLiteral(R"(<li\s+class="active"><span>(\d+)</span></li>)"));
    const QRegularExpressionMatch activeMatch = activePageRe.match(html);
    if (activeMatch.hasMatch()) {
        result.currentPage = activeMatch.captured(1).toInt();
    }

    result.hasPrevious = result.currentPage > 1;
    result.hasNext = html.contains(QStringLiteral("page=%1").arg(result.currentPage + 1));

    static const QRegularExpression pageNumRe(QStringLiteral(R"(search\.php[^"]*page=(\d+))"));
    int maxPage = result.currentPage;
    QRegularExpressionMatchIterator pageIt = pageNumRe.globalMatch(html);
    while (pageIt.hasNext()) {
        maxPage = qMax(maxPage, pageIt.next().captured(1).toInt());
    }
    result.totalPages = qMax(maxPage, result.currentPage);

    return result;
}

OnlineTrack parseSongDetailHtml(const QString& html, const QString& songId)
{
    OnlineTrack track;
    track.songId = songId;
    track.detailUrl = QStringLiteral("https://myfreemp3online.com/song/%1.html").arg(songId);

    static const QRegularExpression streamUrlRe(QStringLiteral(R"(url:\s*'([^']+)')"));
    const QRegularExpressionMatch urlMatch = streamUrlRe.match(html);
    if (urlMatch.hasMatch()) {
        track.streamUrl = urlMatch.captured(1).trimmed();
    }

    static const QRegularExpression coverRe(
        QStringLiteral(R"re(<img\s+class="ue-image"\s+src="([^"]+)")re"));
    const QRegularExpressionMatch coverMatch = coverRe.match(html);
    if (coverMatch.hasMatch()) {
        track.coverUrl = coverMatch.captured(1);
    }

    static const QRegularExpression titleRe(QStringLiteral(R"(title:\s*'([^']*)')"));
    const QRegularExpressionMatch titleMatch = titleRe.match(html);
    if (titleMatch.hasMatch()) {
        track.displayTitle = titleMatch.captured(1);
        splitArtistTitle(track);
    }

    return track;
}
