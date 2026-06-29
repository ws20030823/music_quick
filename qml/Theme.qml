// =============================================================================
// Theme.qml — 浅色主题设计令牌（singleton）
// =============================================================================
// 颜色与尺寸与 Widgets 版 UiTheme::Light 对齐；须在 CMake 中注册 QT_QML_SINGLETON_TYPE
// =============================================================================
pragma Singleton
import QtQuick

QtObject {
    readonly property color bgBase: "#F3F3F7"
    readonly property color bgSidebar: "#E4E8EE"
    readonly property color bgCard: "#FFFFFF"
    readonly property color bgHover: "#0A000000"
    readonly property color accent: "#0078D4"
    readonly property color accentSoft: "#1A0078D4"
    readonly property color textPrimary: "#1B1B1F"
    readonly property color textSecondary: "#6B7280"
    readonly property color textTertiary: "#9CA3AF"
    readonly property color border: "#0F000000"

    readonly property int sidebarWidth: 220
    readonly property int playbackBarHeight: 88
    readonly property int coverSize: 48
    readonly property int listCoverSize: 40
}
