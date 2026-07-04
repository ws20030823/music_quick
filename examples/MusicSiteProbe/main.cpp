// MusicSiteProbe — 控制台学习示例（参考用，默认不参与 CMake 构建）
// 验证请运行 MusicQuickTests（离线 parser + Windows curl 联网测试）
//   MusicSiteProbe.exe [--source myfreemp3|gequbao] [--keyword 周杰伦] [--page 1]
//   MusicSiteProbe.exe [关键词] [页码]   # 兼容旧用法，默认 myfreemp3

#include "network/GequbaoParser.h"
#include "network/MyFreeMp3Parser.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include <QUrlQuery>

namespace {

constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicSiteProbe/1.0";

struct HttpResponse {
    int statusCode = 0;
    QByteArray body;
    QString error;
    bool ok() const { return error.isEmpty() && statusCode >= 200 && statusCode < 300; }
};

class HttpProbe {
public:
    HttpResponse get(const QUrl& url, const QHash<QByteArray, QByteArray>& extraHeaders = {}) const
    {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
        request.setTransferTimeout(20000);
        for (auto it = extraHeaders.constBegin(); it != extraHeaders.constEnd(); ++it) {
            request.setRawHeader(it.key(), it.value());
        }

        QNetworkReply* reply = m_manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        HttpResponse result;
        if (reply->error() != QNetworkReply::NoError) {
            result.error = reply->errorString();
            result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        } else {
            result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            result.body = reply->readAll();
        }
        reply->deleteLater();
        return result;
    }

    HttpResponse post(const QUrl& url,
                      const QByteArray& body,
                      const QHash<QByteArray, QByteArray>& extraHeaders = {}) const
    {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/x-www-form-urlencoded"));
        request.setTransferTimeout(20000);
        for (auto it = extraHeaders.constBegin(); it != extraHeaders.constEnd(); ++it) {
            request.setRawHeader(it.key(), it.value());
        }

        QNetworkReply* reply = m_manager.post(request, body);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        HttpResponse result;
        if (reply->error() != QNetworkReply::NoError) {
            result.error = reply->errorString();
            result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        } else {
            result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            result.body = reply->readAll();
        }
        reply->deleteLater();
        return result;
    }

    HttpResponse head(const QUrl& url, const QHash<QByteArray, QByteArray>& extraHeaders = {}) const
    {
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
        request.setTransferTimeout(20000);
        for (auto it = extraHeaders.constBegin(); it != extraHeaders.constEnd(); ++it) {
            request.setRawHeader(it.key(), it.value());
        }

        QNetworkReply* reply = m_manager.head(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        HttpResponse result;
        result.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError
            && reply->error() != QNetworkReply::ProtocolUnknownError) {
            result.error = reply->errorString();
        }
        reply->deleteLater();
        return result;
    }

private:
    mutable QNetworkAccessManager m_manager;
};

struct ProbeOptions {
    QString source = QStringLiteral("myfreemp3");
    QString keyword = QStringLiteral("周杰伦");
    int page = 1;
};

void printLine(QTextStream& out, const QString& label, const QString& value)
{
    out << label << ": " << value << '\n';
}

ProbeOptions parseArgs(const QStringList& args)
{
    ProbeOptions options;

    for (int i = 0; i < args.size(); ++i) {
        const QString& arg = args.at(i);
        if (arg == QStringLiteral("--source") && i + 1 < args.size()) {
            options.source = args.at(++i);
        } else if ((arg == QStringLiteral("--keyword") || arg == QStringLiteral("-k"))
                   && i + 1 < args.size()) {
            options.keyword = args.at(++i);
        } else if ((arg == QStringLiteral("--page") || arg == QStringLiteral("-p"))
                   && i + 1 < args.size()) {
            options.page = args.at(++i).toInt();
        } else if (!arg.startsWith(QLatin1Char('-'))) {
            if (options.keyword == QStringLiteral("周杰伦")) {
                options.keyword = arg;
            } else {
                options.page = arg.toInt();
            }
        }
    }

    return options;
}

void printCdnResult(QTextStream& out, HttpProbe& http, const QUrl& streamUrl, const QByteArray& referer)
{
    out << "\n[4] 测试 CDN（HEAD）\n";
    const HttpResponse noReferer = http.head(streamUrl);
    const HttpResponse withReferer = http.head(streamUrl, { { "Referer", referer } });
    out << "    无 Referer: HTTP " << noReferer.statusCode << '\n';
    out << "    有 Referer: HTTP " << withReferer.statusCode << '\n';

    out << "\n[5] 集成建议\n";
    const bool refererWorks = withReferer.statusCode >= 200 && withReferer.statusCode < 300;
    const bool directBlocked = noReferer.statusCode == 403;

    if (refererWorks && directBlocked) {
        out << "    ✓ 可集成：搜索 + 详情解析 OK；CDN 需 Referer。\n";
        out << "    → 播放时用 OnlineStreamLoader 预取到本地缓存，或自订 QNetworkAccessManager 带 Referer。\n";
    } else if (refererWorks) {
        out << "    ✓ 可集成：CDN 直接可播或 Referer 后可播。\n";
    } else {
        out << "    ✗ CDN 仍不可播 (HTTP " << withReferer.statusCode << ")，需进一步排查。\n";
    }
    out << '\n';
}

void runMyFreeMp3Probe(const ProbeOptions& options)
{
    QTextStream out(stdout);
    HttpProbe http;

    constexpr auto kBaseUrl = "https://myfreemp3online.com";
    constexpr auto kReferer = "https://myfreemp3online.com/";

    out << "\n=== MusicSiteProbe (MyFreeMp3) ===\n";
    printLine(out, "关键字", options.keyword);
    printLine(out, "页码", QString::number(options.page));

    QUrl searchUrl(QString::fromLatin1(kBaseUrl) + QStringLiteral("/search.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), options.keyword.trimmed());
    if (options.page > 1) {
        query.addQueryItem(QStringLiteral("page"), QString::number(options.page));
    }
    searchUrl.setQuery(query);

    out << "\n[1] GET 搜索页\n    " << searchUrl.toString() << '\n';
    const HttpResponse searchResp = http.get(searchUrl);
    if (!searchResp.ok()) {
        out << "    失败: " << searchResp.error << " (HTTP " << searchResp.statusCode << ")\n";
        out << "\n结论: 搜索入口不可用，无法集成。\n";
        return;
    }
    out << "    HTTP " << searchResp.statusCode << ", 字节数 " << searchResp.body.size() << '\n';

    out << "\n[2] 解析 HTML（MyFreeMp3Parser）\n";
    const SearchPageResult results = parseSearchResultsHtml(QString::fromUtf8(searchResp.body));
    out << "    找到 " << results.tracks.size() << " 首, 当前页 " << results.currentPage
        << ", 总页 " << results.totalPages << '\n';
    if (results.tracks.isEmpty()) {
        out << "\n结论: 搜索成功但无结果，请换关键词。\n";
        return;
    }

    const OnlineTrack& sample = results.tracks.first();
    printLine(out, "    样本 ID", sample.songId);
    printLine(out, "    样本标题", sample.displayTitle);

    out << "\n[3] GET 详情页\n";
    const QUrl detailUrl(sample.detailUrl);
    out << "    " << detailUrl.toString() << '\n';
    const HttpResponse detailResp = http.get(detailUrl);
    if (!detailResp.ok()) {
        out << "    失败: " << detailResp.error << '\n';
        return;
    }
    const OnlineTrack detail = parseSongDetailHtml(QString::fromUtf8(detailResp.body), sample.songId);
    if (detail.streamUrl.isEmpty()) {
        out << "    未找到 url: '...' 播放地址，可能页面结构已变。\n";
        return;
    }
    printLine(out, "    streamUrl", detail.streamUrl);

    printCdnResult(out, http, QUrl(detail.streamUrl), QByteArray(kReferer));
}

void runGequbaoProbe(const ProbeOptions& options)
{
    QTextStream out(stdout);
    HttpProbe http;

    constexpr auto kBaseUrl = "https://www.gequbao.net";
    const QByteArray referer = QByteArray(kBaseUrl) + "/";

    out << "\n=== MusicSiteProbe (歌曲宝 / gequbao) ===\n";
    printLine(out, "关键字", options.keyword);
    printLine(out, "页码", QString::number(options.page));

    QUrl searchUrl(QString::fromLatin1(kBaseUrl)
                   + QStringLiteral("/s/")
                   + QString::fromUtf8(QUrl::toPercentEncoding(options.keyword.trimmed(), "", "/")));
    if (options.page > 1) {
        QUrlQuery pageQuery;
        pageQuery.addQueryItem(QStringLiteral("page"), QString::number(options.page));
        searchUrl.setQuery(pageQuery);
    }

    out << "\n[1] GET 搜索页\n    " << searchUrl.toString() << '\n';
    const HttpResponse searchResp = http.get(searchUrl, { { "Referer", referer } });
    if (!searchResp.ok()) {
        out << "    失败: " << searchResp.error << " (HTTP " << searchResp.statusCode << ")\n";
        out << "\n结论: 搜索入口不可用，无法集成。\n";
        return;
    }
    out << "    HTTP " << searchResp.statusCode << ", 字节数 " << searchResp.body.size() << '\n';

    out << "\n[2] 解析 HTML（GequbaoParser）\n";
    const SearchPageResult results = parseGequbaoSearchHtml(QString::fromUtf8(searchResp.body));
    out << "    找到 " << results.tracks.size() << " 首, 当前页 " << results.currentPage
        << ", 总页 " << results.totalPages << '\n';
    if (results.tracks.isEmpty()) {
        out << "\n结论: 搜索成功但无结果，请换关键词。\n";
        return;
    }

    const OnlineTrack& sample = results.tracks.first();
    printLine(out, "    样本 ID", sample.songId);
    printLine(out, "    样本标题", sample.displayTitle);

    out << "\n[3] GET 详情页 + POST play-url\n";
    const QUrl detailUrl(sample.detailUrl);
    out << "    " << detailUrl.toString() << '\n';
    const HttpResponse detailResp = http.get(detailUrl, { { "Referer", referer } });
    if (!detailResp.ok()) {
        out << "    失败: " << detailResp.error << '\n';
        return;
    }

    const GequbaoMusicDetail detail = parseGequbaoMusicDetailHtml(QString::fromUtf8(detailResp.body));
    if (!detail.isValid()) {
        out << "    未找到 play_id，可能页面结构已变。\n";
        return;
    }
    printLine(out, "    play_id", detail.playId);

    const QUrl playUrlApi(QString::fromLatin1(kBaseUrl) + QStringLiteral("/api/play-url"));
    const QByteArray musicId = detail.mp3Id.isEmpty()
        ? sample.songId.section(QLatin1Char(':'), -1).toUtf8()
        : detail.mp3Id.toUtf8();
    const QByteArray musicReferer = QByteArray(kBaseUrl) + "/music/" + musicId;
    const QByteArray body = QByteArray("id=") + QUrl::toPercentEncoding(detail.playId);
    const HttpResponse playResp = http.post(playUrlApi, body, {
        { "Referer", musicReferer },
    });
    if (!playResp.ok()) {
        out << "    play-url 失败: " << playResp.error << '\n';
        return;
    }

    const QString streamUrl = parseGequbaoPlayUrlJson(QString::fromUtf8(playResp.body));
    if (streamUrl.isEmpty()) {
        out << "    未找到播放地址 JSON 字段 data.url。\n";
        return;
    }
    printLine(out, "    streamUrl", streamUrl);

    printCdnResult(out, http, QUrl(streamUrl), musicReferer);
}

void runProbe(const ProbeOptions& options)
{
    if (options.source == QStringLiteral("gequbao")) {
        runGequbaoProbe(options);
        return;
    }

    if (options.source != QStringLiteral("myfreemp3")) {
        QTextStream(stderr) << "未知 --source: " << options.source
                            << "（支持 myfreemp3、gequbao）\n";
        return;
    }

    runMyFreeMp3Probe(options);
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args.append(QString::fromLocal8Bit(argv[i]));
    }

    runProbe(parseArgs(args));
    return 0;
}
