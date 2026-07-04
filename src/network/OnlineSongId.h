#pragma once

#include <QString>

// =============================================================================
// OnlineSongId — sourceId:trackId 命名空间工具
// =============================================================================

class OnlineSongId final
{
public:
    static QString compose(const QString& sourceId, const QString& trackId);
    static bool parse(const QString& composite, QString* sourceId, QString* trackId);
    static QString normalizeLegacySongId(const QString& songId);
};
