#pragma once

#include <QHash>
#include <QStringList>
#include <QVariantList>

class MusicSourceClient;

// =============================================================================
// MusicSourceRegistry — 资源站注册表（sourceId → client）
// =============================================================================

class MusicSourceRegistry final
{
public:
    void registerSource(MusicSourceClient* client);
    MusicSourceClient* source(const QString& sourceId) const;
    QString displayName(const QString& sourceId) const;
    QStringList sourceIds() const;
    QVariantList toVariantList() const;

private:
    QHash<QString, MusicSourceClient*> m_sources;
};
