#include "network/CurlHttpClient.h"

#include <QFile>
#include <QProcess>
#include <QTemporaryFile>

namespace {

QString curlExecutable()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("curl.exe");
#else
    return QStringLiteral("curl");
#endif
}

QStringList baseArgs(const QString& bodyFilePath)
{
    return {
        QStringLiteral("-s"),
        QStringLiteral("-L"),
        QStringLiteral("--max-time"),
        QStringLiteral("20"),
        QStringLiteral("-o"),
        bodyFilePath,
        QStringLiteral("-w"),
        QStringLiteral("%{http_code}"),
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

    const QByteArray statusBytes = process.readAllStandardOutput().trimmed();
    result.statusCode = QString::fromUtf8(statusBytes).toInt();
    if (result.statusCode <= 0) {
        result.error = QStringLiteral("curl 响应缺少 HTTP 状态码");
        return result;
    }

    const QString bodyPath = args.at(args.indexOf(QStringLiteral("-o")) + 1);
    QFile bodyFile(bodyPath);
    if (!bodyFile.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("无法读取 curl 响应正文");
        return result;
    }
    result.body = bodyFile.readAll();

    if (!result.ok() && result.error.isEmpty()) {
        result.error = QStringLiteral("HTTP %1").arg(result.statusCode);
    }
    return result;
}

HttpResponse runRequest(const QStringList& requestArgs)
{
    QTemporaryFile bodyFile;
    bodyFile.setAutoRemove(true);
    if (!bodyFile.open()) {
        HttpResponse result;
        result.error = QStringLiteral("无法创建临时文件");
        return result;
    }
    bodyFile.close();

    QStringList args = baseArgs(bodyFile.fileName());
    args << requestArgs;
    return runCurl(args);
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
    QStringList requestArgs = headerArgs(headers);
    requestArgs << QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));
    return runRequest(requestArgs);
}

HttpResponse CurlHttpClient::post(const QUrl& url,
                                  const QByteArray& body,
                                  const HeaderList& headers)
{
    QStringList requestArgs;
    requestArgs << QStringLiteral("-X") << QStringLiteral("POST");
    requestArgs << headerArgs(headers);
    requestArgs << QStringLiteral("--data-binary") << QString::fromUtf8(body);
    requestArgs << QString::fromUtf8(url.toEncoded(QUrl::FullyEncoded));
    return runRequest(requestArgs);
}
