// PlaylistPage.qml — 歌单详情（我喜欢的音乐 / 自定义歌单）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    Component.onCompleted: app.refreshActivePlaylist()

    Connections {
        target: app
        function onCurrentPageChanged() {
            if (app.currentPage === 4) {
                app.refreshActivePlaylist()
            }
        }
        function onActivePlaylistIdChanged() {
            if (app.currentPage === 4) {
                app.refreshActivePlaylist()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: Theme.sectionGap

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                spacing: 4
                Text {
                    text: app.activePlaylistName
                    font.pixelSize: 26
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: app.activePlaylistTrackCount > 0
                          ? qsTr("共 %1 首歌曲").arg(app.activePlaylistTrackCount)
                          : qsTr("暂无歌曲，从搜索结果添加")
                    font.pixelSize: 13
                    color: Theme.textSecondary
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                visible: app.activePlaylistId !== "liked"
                text: qsTr("删除歌单")
                onClicked: deleteConfirm.open()
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: parent.down ? "#1AEF4444" : (parent.hovered ? "#0AEF4444" : "transparent")
                    border.color: "#EF4444"
                }
                contentItem: Text {
                    text: parent.text
                    color: "#EF4444"
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        SearchResultList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.activePlaylistTrackCount > 0
            model: app.playlistTrackModel
            showRemoveFromPlaylist: true
            removeFromPlaylistLabel: app.activePlaylistId === "liked"
                                     ? qsTr("取消喜欢")
                                     : qsTr("从歌单移除")
            onRowSelected: function(row) { app.selectPlaylistRow(row) }
            onRowActivated: function(row) { app.playPlaylistRow(row) }
            onLikeClicked: function(row) { app.toggleLikePlaylistRow(row) }
            onRemoveFromPlaylist: function(row) { app.removeTrackFromActivePlaylist(row) }
            onAddToPlaylist: function(row, playlistId) {
                app.playPlaylistRow(row)
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.activePlaylistTrackCount === 0

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width, 360)
                spacing: 12

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 56
                    height: 56
                    radius: 28
                    color: Theme.accentSoft
                    AppIcon {
                        anchors.centerIn: parent
                        name: Icons.heartFilled
                        size: 24
                        color: Theme.accent
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: app.activePlaylistId === "liked"
                          ? qsTr("还没有喜欢的歌曲")
                          : qsTr("歌单是空的")
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("在顶部搜索歌曲，点击 ♥ 或右键加入歌单")
                    font.pixelSize: 13
                    color: Theme.textTertiary
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    CardDialog {
        id: deleteConfirm
        parent: Overlay.overlay
        heading: qsTr("删除歌单")
        description: qsTr("确定删除「%1」？此操作不可恢复。").arg(app.activePlaylistName)
        primaryText: qsTr("删除")
        secondaryText: qsTr("取消")
        destructive: true
        onAccepted: app.deletePlaylist(app.activePlaylistId)
    }
}
