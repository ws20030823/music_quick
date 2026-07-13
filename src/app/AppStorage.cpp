#include "app/AppStorage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

#include <memory>

namespace AppStorage {
namespace {

bool g_initialized = false;
bool g_portable = false;
QString g_dataRoot;
QString g_cacheRoot;
QString g_configIni;

bool detectPortableMarker()
{
    const QString marker = QCoreApplication::applicationDirPath() + QStringLiteral("/portable");
    return QFileInfo::exists(marker);
}

} // namespace

void initialize()
{
    if (g_initialized) {
        return;
    }

    g_portable = detectPortableMarker();
    if (g_portable) {
        g_dataRoot = QDir::cleanPath(
            QCoreApplication::applicationDirPath() + QStringLiteral("/data"));
        g_cacheRoot = g_dataRoot + QStringLiteral("/cache");
        g_configIni = g_dataRoot + QStringLiteral("/config.ini");
        QDir().mkpath(g_dataRoot);
        QDir().mkpath(g_cacheRoot);
    } else {
        g_dataRoot = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        g_cacheRoot = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        if (g_cacheRoot.isEmpty()) {
            g_cacheRoot = QCoreApplication::applicationDirPath() + QStringLiteral("/cache");
        }
        if (!g_dataRoot.isEmpty()) {
            QDir().mkpath(g_dataRoot);
        }
        if (!g_cacheRoot.isEmpty()) {
            QDir().mkpath(g_cacheRoot);
        }
    }

    g_initialized = true;
}

bool isPortable()
{
    if (!g_initialized) {
        initialize();
    }
    return g_portable;
}

QString appDataDir()
{
    if (!g_initialized) {
        initialize();
    }
    return g_dataRoot;
}

QString cacheDir()
{
    if (!g_initialized) {
        initialize();
    }
    return g_cacheRoot;
}

std::unique_ptr<QSettings> createSettings()
{
    if (!g_initialized) {
        initialize();
    }
    if (g_portable) {
        return std::make_unique<QSettings>(g_configIni, QSettings::IniFormat);
    }
    return std::make_unique<QSettings>();
}

} // namespace AppStorage