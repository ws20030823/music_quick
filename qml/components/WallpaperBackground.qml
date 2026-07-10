// WallpaperBackground.qml — 全局背景（白底或壁纸，随不透明度变淡）
import QtQuick
import MusicQuick

Item {
    id: root

    property url wallpaperSource: ""
    property real skinOpacity: 1.0

    readonly property bool hasWallpaper: wallpaperSource.toString().length > 0
    readonly property bool wallpaperReady: wallpaperImage.status === Image.Ready
    readonly property real effectiveOpacity: Theme.mapSkinOpacity(skinOpacity)

    Rectangle {
        anchors.fill: parent
        color: Theme.bgBase
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
