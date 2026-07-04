#pragma once

#include "network/OnlineTrack.h"

#include <QString>

struct GequbaoMusicDetail {
    QString mp3Id;
    QString playId;
    QString title;
    QString artist;
    QString coverUrl;

    bool isValid() const { return !playId.isEmpty(); }
};

SearchPageResult parseGequbaoSearchHtml(const QString& html);
GequbaoMusicDetail parseGequbaoMusicDetailHtml(const QString& html);
QString parseGequbaoPlayUrlJson(const QString& json);
