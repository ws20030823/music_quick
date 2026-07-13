// AppStorage — portable vs installed user-data roots
#pragma once

#include <QSettings>
#include <QString>

#include <memory>

namespace AppStorage {

/// Call once after QCoreApplication exists (and org/app name set).
void initialize();

bool isPortable();

/// Portable: <exe>/data ; Installed: AppDataLocation
QString appDataDir();

/// Portable: <exe>/data/cache ; Installed: CacheLocation
QString cacheDir();

/// Portable Ini under data/config.ini ; Installed: native QSettings (registry on Windows)
std::unique_ptr<QSettings> createSettings();

} // namespace AppStorage
