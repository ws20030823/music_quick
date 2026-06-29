#pragma once

#include "TrackMetadata.h"

#include <QString>

// =============================================================================
// TrackMetadataReader — 本地音频元数据读取
// =============================================================================
// 基于 Qt Multimedia（QMediaPlayer + QMediaMetaData），无额外第三方依赖。
// 采用同步 API + 内部 QEventLoop 等待元数据就绪，适合导入时逐首读取。
// 列表很大时可能阻塞 UI，后续可改为后台线程批量扫描。
// =============================================================================
namespace TrackMetadataReader {

// 从本地文件读取元数据；timeoutMs 防止个别损坏/慢文件长时间卡住主线程
TrackMetadata readFromFile(const QString& filePath, int timeoutMs = 3000);

} // namespace TrackMetadataReader
