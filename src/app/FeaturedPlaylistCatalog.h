#pragma once

#include <QList>
#include <QString>

struct FeaturedPlaylist {
    QString id;
    QString title;
    QString subtitle;
    QString keyword;
    QString coverUrl;
    QString sourceId;
};

class FeaturedPlaylistCatalog
{
public:
    static QList<FeaturedPlaylist> all();
    static const FeaturedPlaylist* find(const QString& id);
};
