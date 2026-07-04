#include "network/CurlHttpClient.h"

#include <QProcess>

namespace {

constexpr auto kStatusMarker = "__HTTP_CODE__:";

QString curlExecutable()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("curl.exe");
#else
    return QStringLiteral("curl");
#endif
}

HttpResponse runCurl(const QStringList& args)
{
    HttpResponse result;

    QProcess process;
    process.setProgram(curlExecutable());
    process.setArguments(args);
    process.start();

    if (!process.waitForStarted(5000)) {
        result.error = QStringLiteral("无法启动 curl");
        return result;
    }
    if (!process.waitForFinished(30000)) {
        process.kill();
        result.error = QStringLiteral("curl 请求超时");
        return result;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString stderrText = QString::fromUtf8(process.readAllStandardError()).trimmed();
        result.error = stderrText.isEmpty() ? QStringLiteral("curl 退出码 %1").arg(process.exitCode())
                                            : stderrText;
        return result;
    }

    QByteArray output = process.readAllStandardOutput();
    const int markerIndex = output.lastIndexOf(kStatusMarker);
    if (markerIndex >= 0) {
        const QByteArray statusBytes = output.mid(markerIndex + int(sizeof(kStatusMarker) - 1)).trimmed();
        result.statusCode = QString::fromUtf8(statusBytes).toInt();
        output.truncate(markerIndex);
    } else {
        result.error = QStringLiteral("curl 响应缺少 HTTP 状态码");
        return result;
    }

    result.body = output;
    if (result.statusCode >= 400) {
        result.error = QStringLiteral("HTTP %1").arg(result.statusCode);
    }
    return result;
}

QStringList baseArgs()
{
    return {
        QStringLiteral("-s"),
        QStringLiteral("-L"),
        QStringLiteral("--max-time"),
        QStringLiteral("20"),
        QStringLiteral("-w"),
        QStringLiteral("\n%1%{http_code}").arg(QString::fromLatin1(kStatusMarker)),
    };
}

QStringList headerArgs(const CurlHttpClient::HeaderList& headers)
{
    QStringList args;
    for (const auto& header : headers) {
        args << QStringLiteral("-H")
             << QString::fromUtf8(header.first + ": " + header.second);
    }
    return args;
}

} // namespace

bool CurlHttpClient::isAvailable()
{
    QProcess process;
    process.setProgram(curlExecutable());
    process.setArguments({QStringLiteral("--version")});
    process.start();
    if (!process.waitForStarted(2000)) {
        return false;
    }
    process.waitForFinished(5000);
    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

HttpResponse CurlHttpClient::get(const QUrl& url, const HeaderList& headers)
{
    QStringList args = baseArgs();
    args << headerArgs(headers);
    args << QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));
    return runCurl(args);
}

HttpResponse CurlHttpClient::post(const QUrl& url,
                                  const QByteArray& body,
                                  const HeaderList& headers)
{
    QStringList args = baseArgs();
    args << QStringLiteral("-X") << QStringLiteral("POST");
    args << headerArgs(headers);
    args << QStringLiteral("--data-binary") << QString::fromUtf8(body);
    args << QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));
    return runCurl(args);
}
