// DiscoverPage.qml — 首页（3D 球面歌单菜单）
import QtQuick
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    Rectangle {
        id: menuStage
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        radius: Theme.radiusLg
        clip: true
        color: Theme.bgCard

        InfiniteMenu {
            id: menu
            anchors.fill: parent
            items: app.featuredPlaylists
            menuScale: 1.0
            immersiveDark: false
            onItemActivated: function(id) {
                app.openFeaturedPlaylist(id)
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
            text: qsTr("拖动旋转 · 点击下方按钮进入歌单")
            font.pixelSize: 12
            color: Theme.textTertiary
            z: 80
            opacity: menu.isMoving ? 0 : 0.85

            Behavior on opacity {
                enabled: !Theme.reduceMotion
                NumberAnimation { duration: 300 }
            }
        }
    }
}
