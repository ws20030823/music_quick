#include "network/OnlineSongId.h"

namespace {

bool isPureNumericId(const QString& songId)
{
    if (songId.isEmpty()) {
        return false;
    }

    for (const QChar ch : songId) {
        if (!ch.isDigit()) {
            return false;
        }
    }

    return true;
}

} // namespace

QString OnlineSongId::compose(const QString& sourceId, const QString& trackId)
{
    return sourceId + QLatin1Char(':') + trackId;
}

bool OnlineSongId::parse(const QString& composite, QString* sourceId, QString* trackId)
{
    const int separatorIndex = composite.indexOf(QLatin1Char(':'));
    if (separatorIndex <= 0 || separatorIndex >= composite.size() - 1) {
        return false;
    }

    if (sourceId != nullptr) {
        *sourceId = composite.left(separatorIndex);
    }

    if (trackId != nullptr) {
        *trackId = composite.mid(separatorIndex + 1);
    }

    return true;
}

QString OnlineSongId::normalizeLegacySongId(const QString& songId)
{
    if (songId.startsWith(QStringLiteral("local:"))) {
        return songId;
    }

    if (songId.contains(QLatin1Char(':'))) {
        return songId;
    }

    if (isPureNumericId(songId)) {
        return compose(QStringLiteral("myfreemp3"), songId);
    }

    return songId;
}
