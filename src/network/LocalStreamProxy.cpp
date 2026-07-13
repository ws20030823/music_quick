#include "network/LocalStreamProxy.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHostAddress>
#include <QTcpSocket>
#include <QUuid>

namespace {

constexpr auto kDefaultUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MusicQuick/1.0";

QByteArray headerValue(const QByteArray& request, const QByteArray& name)
{
    const QList<QByteArray> lines = request.split('\n');
    const QByteArray prefix = name.toLower() + ':';
    for (QByteArray line : lines) {
        line = line.trimmed();
        if (line.toLower().startsWith(prefix)) {
            return line.mid(prefix.size()).trimmed();
        }
    }
    return {};
}

QByteArray reasonPhrase(int status)
{
    switch (status) {
    case 200:
        return "OK";
    case 206:
        return "Partial Content";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 502:
        return "Bad Gateway";
    default:
        return "OK";
    }
}

void writeSimpleResponse(QTcpSocket* socket, int status, const QByteArray& body)
{
    socket->write("HTTP/1.1 " + QByteArray::number(status) + ' ' + reasonPhrase(status) + "\r\n");
    socket->write("Connection: close\r\n");
    socket->write("Content-Type: text/plain; charset=utf-8\r\n");
    socket->write("Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n");
    socket->write(body);
    socket->disconnectFromHost();
}

} // namespace

LocalStreamProxy::LocalStreamProxy(QObject* parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection, this, &LocalStreamProxy::handleNewConnection);
}

QUrl LocalStreamProxy::streamUrl(const QUrl& upstreamUrl, const StreamFetchOptions& options)
{
    if (!upstreamUrl.isValid() || !ensureListening()) {
        return {};
    }

    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_streams.insert(token, {upstreamUrl, options});

    QUrl local;
    local.setScheme(QStringLiteral("http"));
    local.setHost(QStringLiteral("127.0.0.1"));
    local.setPort(m_server.serverPort());
    local.setPath(QStringLiteral("/stream/%1").arg(token));
    return local;
}

bool LocalStreamProxy::isListening() const
{
    return m_server.isListening();
}

bool LocalStreamProxy::ensureListening()
{
    if (m_server.isListening()) {
        return true;
    }
    return m_server.listen(QHostAddress::LocalHost, 0);
}

void LocalStreamProxy::handleNewConnection()
{
    while (QTcpSocket* socket = m_server.nextPendingConnection()) {
        m_clients.insert(socket, {});
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            handleClientReadyRead(socket);
        });
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            handleClientDisconnected(socket);
        });
    }
}

void LocalStreamProxy::handleClientReadyRead(QTcpSocket* socket)
{
    if (!m_clients.contains(socket)) {
        socket->disconnectFromHost();
        return;
    }

    ClientState& state = m_clients[socket];
    state.request += socket->readAll();
    if (!state.request.contains("\r\n\r\n")) {
        return;
    }

    const QList<QByteArray> lines = state.request.split('\n');
    if (lines.isEmpty()) {
        writeSimpleResponse(socket, 400, "bad request");
        return;
    }

    const QList<QByteArray> parts = lines.first().trimmed().split(' ');
    if (parts.size() < 2) {
        writeSimpleResponse(socket, 400, "bad request");
        return;
    }

    const QByteArray method = parts.at(0).trimmed();
    if (method != "GET" && method != "HEAD") {
        writeSimpleResponse(socket, 405, "method not allowed");
        return;
    }

    const QByteArray path = parts.at(1);
    const QByteArray prefix = "/stream/";
    if (!path.startsWith(prefix)) {
        writeSimpleResponse(socket, 404, "not found");
        return;
    }

    const QString token = QString::fromUtf8(path.mid(prefix.size()).split('?').first());
    startUpstream(socket, token, method, headerValue(state.request, "Range"));
}

void LocalStreamProxy::handleClientDisconnected(QTcpSocket* socket)
{
    ClientState state = m_clients.take(socket);
    if (state.upstreamReply) {
        state.upstreamReply->abort();
    }
    socket->deleteLater();
}

void LocalStreamProxy::startUpstream(QTcpSocket* socket,
                                     const QString& token,
                                     const QByteArray& method,
                                     const QByteArray& rangeHeader)
{
    const auto it = m_streams.constFind(token);
    if (it == m_streams.constEnd()) {
        writeSimpleResponse(socket, 404, "stream expired");
        return;
    }

    QNetworkRequest request(it->upstreamUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      it->options.userAgent.isEmpty()
                          ? QString::fromLatin1(kDefaultUserAgent)
                          : it->options.userAgent);
    if (!it->options.referer.isEmpty()) {
        request.setRawHeader("Referer", it->options.referer.toUtf8());
    }
    for (auto headerIt = it->options.rawHeaders.cbegin(); headerIt != it->options.rawHeaders.cend(); ++headerIt) {
        request.setRawHeader(headerIt.key(), headerIt.value());
    }
    if (!rangeHeader.isEmpty()) {
        request.setRawHeader("Range", rangeHeader);
    }
    request.setRawHeader("Accept-Encoding", "identity");
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setTransferTimeout(60000);

    QNetworkReply* reply = method == "HEAD" ? m_network.head(request) : m_network.get(request);
    m_clients[socket].upstreamReply = reply;

    connect(reply, &QNetworkReply::metaDataChanged, this, [this, socket, reply, method]() {
        if (!m_clients.contains(socket) || !socket->isOpen()) {
            return;
        }
        if (socket->property("headersWritten").toBool()) {
            return;
        }
        socket->setProperty("headersWritten", true);
        socket->write(responseHeaderFor(reply, method == "HEAD"));
        if (method == "HEAD") {
            socket->disconnectFromHost();
        }
    });

    connect(reply, &QNetworkReply::readyRead, this, [this, socket, reply, method]() {
        if (socket->isOpen()) {
            if (!socket->property("headersWritten").toBool()) {
                socket->setProperty("headersWritten", true);
                socket->write(responseHeaderFor(reply, method == "HEAD"));
            }
            socket->write(reply->readAll());
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, socket, reply, method]() {
        reply->deleteLater();
        if (!m_clients.contains(socket) || !socket->isOpen()) {
            return;
        }
        if (reply->error() != QNetworkReply::NoError
            && reply->error() != QNetworkReply::OperationCanceledError) {
            if (!socket->property("headersWritten").toBool()) {
                writeSimpleResponse(socket, 502, reply->errorString().toUtf8());
            } else {
                socket->disconnectFromHost();
            }
            return;
        }
        if (!socket->property("headersWritten").toBool()) {
            socket->setProperty("headersWritten", true);
            socket->write(responseHeaderFor(reply, method == "HEAD"));
        }
        if (method != "HEAD") {
            socket->write(reply->readAll());
        }
        socket->disconnectFromHost();
    });
}

QByteArray LocalStreamProxy::responseHeaderFor(QNetworkReply* reply, bool headOnly) const
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const int effectiveStatus = status > 0 ? status : 200;

    QByteArray headers;
    headers += "HTTP/1.1 " + QByteArray::number(effectiveStatus) + ' ' + reasonPhrase(effectiveStatus) + "\r\n";
    headers += "Connection: close\r\n";

    const QList<QByteArray> passHeaders = {
        "content-type",
        "content-length",
        "content-range",
        "accept-ranges",
        "cache-control",
    };
    const auto rawHeaders = reply->rawHeaderPairs();
    for (const QByteArray& wantedName : passHeaders) {
        for (const auto& pair : rawHeaders) {
            if (pair.first.toLower() == wantedName) {
                headers += pair.first + ": " + pair.second + "\r\n";
                break;
            }
        }
    }
    if (headOnly && !reply->hasRawHeader("content-length")) {
        headers += "Content-Length: 0\r\n";
    }
    headers += "\r\n";
    return headers;
}
