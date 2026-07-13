// PlaylistPage.qml — 歌单详情（我喜欢的音乐 / 自定义歌单）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    signal importPlaylistClicked()

    property bool batchDeleteMode: false
    property int batchSelectedCount: playlistList.selectedRows.length

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
                visible: app.activePlaylistTrackCount > 0 && !root.batchDeleteMode
                text: qsTr("批量删除")
                onClicked: root.batchDeleteMode = true
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: parent.hovered ? Theme.bgHover : "transparent"
                    border.color: Theme.borderStrong
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textSecondary
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                visible: root.batchDeleteMode
                text: qsTr("取消")
                onClicked: {
                    root.batchDeleteMode = false
                    playlistList.clearBatchSelection()
                }
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: parent.hovered ? Theme.bgHover : "transparent"
                    border.color: Theme.borderStrong
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textSecondary
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                visible: root.batchDeleteMode
                enabled: root.batchSelectedCount > 0
                text: qsTr("删除所选%1").arg(root.batchSelectedCount > 0 ? " (" + root.batchSelectedCount + ")" : "")
                onClicked: batchDeleteConfirm.open()
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: !parent.enabled ? "#12000000"
                         : (parent.hovered ? "#1AEF4444" : "transparent")
                    border.color: parent.enabled ? "#EF4444" : Theme.borderStrong
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? "#EF4444" : Theme.textTertiary
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                visible: !root.batchDeleteMode
                text: qsTr("导入歌单")
                onClicked: root.importPlaylistClicked()
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: parent.down ? Theme.accent
                         : (parent.hovered ? "#1689E8" : Theme.accent)
                }
                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                visible: app.activePlaylistId !== "liked" && !root.batchDeleteMode
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
            id: playlistList
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.activePlaylistTrackCount > 0
            model: app.playlistTrackModel
            batchMode: root.batchDeleteMode
            showRemoveFromPlaylist: true
            removeFromPlaylistLabel: app.activePlaylistId === "liked"
                                     ? qsTr("取消喜欢")
                                     : qsTr("从歌单移除")
            onRowSelected: function(row) { app.selectPlaylistRow(row) }
            onRowActivated: function(row) { app.playPlaylistRow(row) }
            onLikeClicked: function(row) { app.toggleLikePlaylistRow(row) }
            onRemoveFromPlaylist: function(row) { app.removeTrackFromActivePlaylist(row) }
            onSelectionChanged: root.batchSelectedCount = playlistList.selectedRows.length
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
                    text: qsTr("在顶部搜索歌曲加入歌单，或从网易云 / QQ 音乐歌单链接导入")
                    font.pixelSize: 13
                    color: Theme.textTertiary
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("导入外部歌单")
                    onClicked: root.importPlaylistClicked()
                    background: Rectangle {
                        radius: 18
                        color: parent.hovered ? "#1689E8" : Theme.accent
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
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

    CardDialog {
        id: batchDeleteConfirm
        parent: Overlay.overlay
        heading: qsTr("批量删除歌曲")
        description: qsTr("确定从「%1」删除选中的 %2 首歌曲？")
                     .arg(app.activePlaylistName)
                     .arg(root.batchSelectedCount)
        primaryText: qsTr("删除")
        secondaryText: qsTr("取消")
        destructive: true
        onAccepted: {
            app.removeTracksFromActivePlaylist(playlistList.selectedRows)
            playlistList.clearBatchSelection()
            root.batchDeleteMode = false
        }
    }
}
