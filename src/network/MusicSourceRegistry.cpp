#include "network/MusicSourceRegistry.h"



#include "network/MusicSourceClient.h"



#include <QVariantMap>



void MusicSourceRegistry::registerSource(MusicSourceClient* client)

{

    if (client == nullptr) {

        return;

    }



    m_sources.insert(client->sourceId(), client);

}



MusicSourceClient* MusicSourceRegistry::source(const QString& sourceId) const

{

    return m_sources.value(sourceId, nullptr);

}



QString MusicSourceRegistry::displayName(const QString& sourceId) const

{

    if (MusicSourceClient* client = source(sourceId)) {

        return client->displayName();

    }



    return {};

}



QStringList MusicSourceRegistry::sourceIds() const

{

    return m_sources.keys();

}



QVariantList MusicSourceRegistry::toVariantList() const

{

    QVariantList items;



    for (const QString& sourceId : sourceIds()) {

        QVariantMap entry;

        entry.insert(QStringLiteral("sourceId"), sourceId);

        entry.insert(QStringLiteral("name"), displayName(sourceId));

        items.append(entry);

    }



    return items;

}

