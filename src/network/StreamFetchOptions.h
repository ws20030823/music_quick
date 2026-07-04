#pragma once

#include <QByteArray>
#include <QMap>
#include <QString>

// =============================================================================
// StreamFetchOptions — 在线音频预取时的 HTTP 上下文
// =============================================================================

struct StreamFetchOptions {
    QString referer;
    QString userAgent;
    QMap<QByteArray, QByteArray> rawHeaders;
};
