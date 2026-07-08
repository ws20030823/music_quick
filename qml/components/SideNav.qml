// SideNav.qml — 左侧导航（发现 / 我的音乐 / 歌单）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Rectangle {
    id: root
    color: "transparent"
    implicitWidth: Theme.sidebarWidth

    property int currentPage: 0
    signal navigate(int page)

    CreatePlaylistDialog {
        id: createPlaylistDialog
        parent: Overlay.overlay
    }

    ScrollView {
        id: navScroll
        anchors.fill: parent
        anchors.margins: 12
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        Column {
            width: navScroll.availableWidth > 0 ? navScroll.availableWidth : root.width - 24
            spacing: 4

            SectionLabel { title: qsTr("发现") }
            NavButton { label: qsTr("首页"); page: 0; playlistId: ""; featuredPlaylistId: "" }
            NavButton { label: qsTr("推荐"); page: 1; playlistId: ""; featuredPlaylistId: "" }

            SectionLabel { title: qsTr("我的音乐") }
            NavButton { label: qsTr("本地音乐"); page: 2; playlistId: ""; featuredPlaylistId: "" }
            NavButton {
                label: qsTr("最近播放")
                page: -1
                enabled: false
            }

            SectionLabel {
                visible: app.favoriteFeaturedPlaylists.length > 0
                title: qsTr("收藏歌单")
            }

            Repeater {
                model: app.favoriteFeaturedPlaylists
                delegate: NavButton {
                    required property var modelData
                    label: modelData.title
                    page: 5
                    playlistId: ""
                    featuredPlaylistId: modelData.id
                    subtitle: modelData.subtitle
                }
            }

            Row {
                width: parent.width
                topPadding: 14
                bottomPadding: 4
                spacing: 4

                Text {
                    width: parent.width - addPlaylistBtn.width - parent.spacing
                    text: qsTr("创建的歌单")
                    font.pixelSize: 11
                    color: Theme.textTertiary
                    leftPadding: 4
                    elide: Text.ElideRight
                }

                ToolButton {
                    id: addPlaylistBtn
                    implicitWidth: 28
                    implicitHeight: 28
                    ToolTip.text: qsTr("新建歌单")
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.bgHover : "transparent"
                    }
                    contentItem: Text {
                        text: "+"
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: createPlaylistDialog.open()
                }
            }

            Repeater {
                model: app.sidebarPlaylists
                delegate: NavButton {
                    required property var modelData
                    label: modelData.name
                    page: 4
                    playlistId: modelData.id
                    subtitle: modelData.trackCount > 0
                             ? qsTr("%1 首").arg(modelData.trackCount) : ""
                    showDelete: !modelData.builtin
                    onDeleteClicked: app.deletePlaylist(modelData.id)
                }
            }
        }
    }

    component SectionLabel: Text {
        required property string title
        width: parent.width
        text: title
        font.pixelSize: 11
        color: Theme.textTertiary
        topPadding: 14
        bottomPadding: 4
        leftPadding: 4
    }

    component NavButton: Button {
        id: navBtn
        property string label
        property int page
        property string playlistId: ""
        property string featuredPlaylistId: ""
        property string subtitle: ""
        property bool showDelete: false
        signal deleteClicked()

        width: parent.width
        enabled: navBtn.page >= 0
        opacity: enabled ? 1 : 0.45
        flat: true
        checkable: enabled
        checked: enabled && root.currentPage === page
                 && ((featuredPlaylistId !== "" && app.activeFeaturedPlaylistId === featuredPlaylistId)
                     || (featuredPlaylistId === "" && playlistId === "" && app.activeFeaturedPlaylistId === "")
                     || (playlistId !== "" && app.activePlaylistId === playlistId))
        onClicked: {
            if (page < 0)
                return
            if (featuredPlaylistId !== "") {
                app.openFeaturedPlaylist(featuredPlaylistId)
            } else if (playlistId !== "") {
                app.openPlaylist(playlistId)
            } else {
                root.navigate(page)
            }
        }

        background: Item {
            id: navBg
            implicitHeight: navBtn.subtitle !== "" ? 48 : 40

            Rectangle {
                anchors.fill: parent
                radius: Theme.radiusMd
                color: navBg.parent.checked ? Theme.accentSoft
                     : (navBg.parent.hovered && navBg.parent.enabled ? Theme.bgHover : "transparent")
            }

            Rectangle {
                width: navBg.parent.checked ? 3 : 0
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                radius: 2
                color: Theme.accent
            }

            ToolButton {
                visible: navBtn.showDelete && navBtn.hovered
                anchors.right: parent.right
                anchors.rightMargin: 4
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 24
                implicitHeight: 24
                onClicked: navBtn.deleteClicked()
                background: Rectangle {
                    radius: 12
                    color: parent.hovered ? "#1AEF4444" : "transparent"
                }
                contentItem: Text {
                    text: "×"
                    font.pixelSize: 14
                    color: "#EF4444"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        contentItem: Item {
            implicitHeight: navBtn.subtitle !== "" ? 48 : 40
            width: navBtn.width

            Text {
                anchors.left: parent.left
                anchors.leftMargin: navBtn.checked ? 10 : 12
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: navBtn.subtitle !== "" ? -8 : 0
                anchors.right: parent.right
                anchors.rightMargin: navBtn.showDelete ? 32 : 12
                text: navBtn.label
                color: navBtn.checked ? Theme.accent : Theme.textSecondary
                font.pixelSize: 14
                font.weight: navBtn.checked ? Font.DemiBold : Font.Normal
                elide: Text.ElideRight
            }

            Text {
                visible: navBtn.subtitle !== ""
                anchors.left: parent.left
                anchors.leftMargin: navBtn.checked ? 10 : 12
                anchors.top: parent.top
                anchors.topMargin: 26
                text: navBtn.subtitle
                color: Theme.textTertiary
                font.pixelSize: 11
            }
        }
    }
}
