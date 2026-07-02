#pragma once

#include "network/OnlineTrack.h"

#include <QString>

SearchPageResult parseSearchResultsHtml(const QString& html);
OnlineTrack parseSongDetailHtml(const QString& html, const QString& songId);
QString stripHtmlTags(const QString& htmlFragment);
