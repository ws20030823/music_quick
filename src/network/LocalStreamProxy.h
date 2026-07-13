#pragma once

#include "network/StreamFetchOptions.h"

#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <QUrl>

class QNetworkReply;
class QTcpSocket;

class LocalStreamProxy final : public QObject
{
    Q_OBJECT

public:
    explicit LocalStreamProxy(QObject* parent = nullptr);

    QUrl streamUrl(const QUrl& upstreamUrl, const StreamFetchOptions& options);
    bool isListening() const;

private:
    struct StreamContext {
        QUrl upstreamUrl;
        StreamFetchOptions options;
    };

    struct ClientState {
        QByteArray request;
        QPointer<QNetworkReply> upstreamReply;
    };

    bool ensureListening();
    void handleNewConnection();
    void handleClientReadyRead(QTcpSocket* socket);
    void handleClientDisconnected(QTcpSocket* socket);
    void startUpstream(QTcpSocket* socket,
                       const QString& token,
                       const QByteArray& method,
                       const QByteArray& rangeHeader);
    QByteArray responseHeaderFor(QNetworkReply* reply, bool headOnly) const;

    QTcpServer m_server;
    QNetworkAccessManager m_network;
    QHash<QString, StreamContext> m_streams;
    QHash<QTcpSocket*, ClientState> m_clients;
};
