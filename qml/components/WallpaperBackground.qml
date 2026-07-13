// WallpaperBackground.qml — 全局背景（白底或壁纸，仅壁纸层受不透明度控制）
import QtQuick
import MusicQuick

Item {
    id: root

    property url wallpaperSource: ""
    property real wallpaperOpacity: 1.0

    readonly property bool hasWallpaper: wallpaperSource.toString().length > 0
    readonly property bool wallpaperReady: wallpaperImage.status === Image.Ready
    readonly property real effectiveOpacity: Theme.mapWallpaperOpacity(wallpaperOpacity)

    Rectangle {
        anchors.fill: parent
        color: Theme.bgCard
        opacity: root.effectiveOpacity
        visible: !hasWallpaper
    }

    Image {
        id: wallpaperImage
        anchors.fill: parent
        source: root.wallpaperSource
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
        mipmap: true
        opacity: root.effectiveOpacity
        visible: hasWallpaper && wallpaperReady
    }

    Connections {
        target: wallpaperImage
        function onStatusChanged() {
            if (wallpaperImage.status === Image.Error)
                console.warn("WallpaperBackground: failed to load", root.wallpaperSource.toString())
        }
    }
}
