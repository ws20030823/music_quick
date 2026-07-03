// MusicSiteProbe — 控制台學習範例：用 C++ 驗證「搜歌 + 解析 + 播放 URL」
//
// 對應 Python 腳本 scripts/learn_music_site_probe.py 的五步流程。
// 編譯：在 Qt Creator 中重新 Configure，會出現 MusicSiteProbe 目標。
// 執行：MusicSiteProbe.exe [關鍵字] [頁碼]
//
// 建議學習順序：
//   1. 先讀 HttpProbe 類（同步 HTTP，比 GUI 的 async 好理解）
//   2. 再讀 runProbe() 五步
//   3. 對照 src/network/MyFreeMp3Parser.cpp 的正則
//   4. 對照 src/network/MyFreeMp3Client.cpp 的 async 版本
//   5. 對照 src/network/OnlineStreamLoader.cpp 的 Referer 預取

#include "network/MyFreeMp3Parser.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include <QUrlQuery>

namespace {

constexpr auto kBaseUrl = "https://myfreemp3online.com";
constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicSiteProbe/1.0";
constexpr auto kReferer = "https://myfreemp3online.com/";

// ---------------------------------------------------------------------------
// 同步 HTTP 小工具（教學用；GUI 專案請用 MyFreeMp3Client 的非同步寫法）
// ---------------------------------------------------------------------------
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

    /** HEAD 請求，只取狀態碼（用於測 CDN 是否 403） */
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
            // HEAD 有時回 ProtocolUnknownError 但 statusCode 仍可用
            result.error = reply->errorString();
        }
        reply->deleteLater();
        return result;
    }

private:
    mutable QNetworkAccessManager m_manager;
};

QUrl buildSearchUrl(const QString& keyword, int page)
{
    QUrl url(QString::fromLatin1(kBaseUrl) + QStringLiteral("/search.php"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("q"), keyword.trimmed());
    if (page > 1) {
        query.addQueryItem(QStringLiteral("page"), QString::number(page));
    }
    url.setQuery(query);
    return url;
}

void printLine(QTextStream& out, const QString& label, const QString& value)
{
    out << label << ": " << value << '\n';
}

// ---------------------------------------------------------------------------
// 五步探測（與 Python 腳本一一對應）
// ---------------------------------------------------------------------------
void runProbe(const QString& keyword, int page)
{
    QTextStream out(stdout);
    HttpProbe http;

    out << "\n=== MusicSiteProbe (C++) ===\n";
    printLine(out, "關鍵字", keyword);
    printLine(out, "頁碼", QString::number(page));

    // Step 1: 搜索入口是否可訪問
    const QUrl searchUrl = buildSearchUrl(keyword, page);
    out << "\n[1] GET 搜索頁\n    " << searchUrl.toString() << '\n';
    const HttpResponse searchResp = http.get(searchUrl);
    if (!searchResp.ok()) {
        out << "    失敗: " << searchResp.error << " (HTTP " << searchResp.statusCode << ")\n";
        out << "\n結論: 搜索入口不可用，無法集成。\n";
        return;
    }
    out << "    HTTP " << searchResp.statusCode << ", 字節數 " << searchResp.body.size() << '\n';

    // Step 2: 從 HTML 解析歌曲列表
    out << "\n[2] 解析 HTML（MyFreeMp3Parser）\n";
    const SearchPageResult results = parseSearchResultsHtml(QString::fromUtf8(searchResp.body));
    out << "    找到 " << results.tracks.size() << " 首, 當前頁 " << results.currentPage
        << ", 總頁 " << results.totalPages << '\n';
    if (results.tracks.isEmpty()) {
        out << "\n結論: 搜索成功但無結果，請換關鍵字。\n";
        return;
    }
    const OnlineTrack& sample = results.tracks.first();
    printLine(out, "    樣本 ID", sample.songId);
    printLine(out, "    樣本標題", sample.displayTitle);

    // Step 3: 請求詳情頁，提取 APlayer stream URL
    out << "\n[3] GET 詳情頁\n";
    const QUrl detailUrl(sample.detailUrl);
    out << "    " << detailUrl.toString() << '\n';
    const HttpResponse detailResp = http.get(detailUrl);
    if (!detailResp.ok()) {
        out << "    失敗: " << detailResp.error << '\n';
        return;
    }
    const OnlineTrack detail = parseSongDetailHtml(QString::fromUtf8(detailResp.body), sample.songId);
    if (detail.streamUrl.isEmpty()) {
        out << "    未找到 url: '...' 播放地址，可能頁面結構已變。\n";
        return;
    }
    printLine(out, "    streamUrl", detail.streamUrl);

    // Step 4: 對比無 Referer / 有 Referer
    out << "\n[4] 測試 CDN（HEAD）\n";
    const QUrl streamUrl(detail.streamUrl);
    const HttpResponse noReferer = http.head(streamUrl);
    const HttpResponse withReferer = http.head(streamUrl, {
        { "Referer", kReferer },
    });
    out << "    無 Referer: HTTP " << noReferer.statusCode << '\n';
    out << "    有 Referer: HTTP " << withReferer.statusCode << '\n';

    // Step 5: 結論
    out << "\n[5] 集成建議\n";
    const bool refererWorks = withReferer.statusCode >= 200 && withReferer.statusCode < 300;
    const bool directBlocked = noReferer.statusCode == 403;

    if (refererWorks && directBlocked) {
        out << "    ✓ 可集成：搜索 + 詳情解析 OK；CDN 需 Referer。\n";
        out << "    → 播放時用 OnlineStreamLoader 預取到本地快取，或自訂 QNetworkAccessManager 帶 Referer。\n";
        out << "    → 勿直接用 QMediaPlayer::setSource(https://...) 不帶 Referer。\n";
    } else if (refererWorks) {
        out << "    ✓ 可集成：CDN 直接可播或 Referer 後可播。\n";
    } else {
        out << "    ✗ CDN 仍不可播 (HTTP " << withReferer.statusCode << ")，需進一步排查。\n";
    }
    out << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    const QString keyword = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("周杰倫");
    const int page = argc > 2 ? QString::fromLocal8Bit(argv[2]).toInt() : 1;

    runProbe(keyword, page);
    return 0;
}
