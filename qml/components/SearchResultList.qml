// SearchResultList.qml — 网易云风格搜索结果表格（标题/专辑/喜欢/时长 + 右键菜单）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    property alias model: listView.model
    property bool showRemoveFromPlaylist: false
    property string removeFromPlaylistLabel: qsTr("从歌单移除")
    property bool batchMode: false
    property var selectedRows: []
    signal rowSelected(int row)
    signal rowActivated(int row)
    signal likeClicked(int row)
    signal addToPlaylist(int row, string playlistId)
    signal removeFromPlaylist(int row)
    signal selectionChanged()

    property int contextRow: -1

    function isRowBatchSelected(row) {
        return selectedRows.indexOf(row) >= 0
    }

    function clearBatchSelection() {
        selectedRows = []
        selectionChanged()
    }

    function toggleBatchRow(row) {
        const copy = selectedRows.slice()
        const idx = copy.indexOf(row)
        if (idx >= 0)
            copy.splice(idx, 1)
        else
            copy.push(row)
        selectedRows = copy
        selectionChanged()
    }

    onBatchModeChanged: {
        if (!batchMode)
            clearBatchSelection()
    }

    // ── 表头 ──
    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.searchTableHeaderHeight
        color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.pagePadding
                anchors.rightMargin: Theme.pagePadding
                spacing: 12

                Text {
                    text: "#"
                    Layout.preferredWidth: Theme.searchIndexWidth
                    color: Theme.textTertiary
                    font.pixelSize: 12
                }
                Text {
                    text: qsTr("标题")
                    Layout.fillWidth: true
                    color: Theme.textTertiary
                    font.pixelSize: 12
                }
                Text {
                    text: qsTr("专辑")
                    Layout.preferredWidth: Theme.searchAlbumWidth
                    color: Theme.textTertiary
                    font.pixelSize: 12
                }
                Item { Layout.preferredWidth: Theme.searchLikeWidth }
                Text {
                    text: qsTr("时长")
                    Layout.preferredWidth: Theme.searchDurationWidth
                    horizontalAlignment: Text.AlignRight
                    color: Theme.textTertiary
                    font.pixelSize: 12
                }
            }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Theme.border
        }
    }

    ListView {
            id: listView
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            clip: true
            spacing: 0

            delegate: Rectangle {
                id: rowItem
                required property int index
                required property string title
                required property string artist
                required property string album
                required property string duration
                required property bool isPlaying
                required property bool isLiked
                required property bool isSelected
                required property string songId

                width: ListView.view.width
                height: Theme.searchRowHeight
                color: isPlaying || isSelected ? Theme.accentSoft
                     : (rowMouse.containsMouse ? Theme.bgHover : "transparent")

                Behavior on color { ColorAnimation { duration: 150 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.pagePadding
                    anchors.rightMargin: Theme.pagePadding
                    spacing: 12

                    CheckBox {
                        visible: root.batchMode
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        checked: root.isRowBatchSelected(index)
                        onClicked: root.toggleBatchRow(index)
                    }

                    // 序号 / 播放态
                    Item {
                        Layout.preferredWidth: Theme.searchIndexWidth
                        Layout.preferredHeight: 28
                        Text {
                            anchors.centerIn: parent
                            visible: !isPlaying && !isSelected
                            text: index < 9 ? ("0" + (index + 1)) : String(index + 1)
                            color: Theme.textTertiary
                            font.pixelSize: 12
                        }
                        AppIcon {
                            anchors.centerIn: parent
                            visible: isPlaying || isSelected
                            name: Icons.play
                            size: 14
                            color: Theme.accent
                        }
                    }

                    // 标题 + 歌手
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: title
                            font.pixelSize: 14
                            color: isPlaying || isSelected ? Theme.accent : Theme.textPrimary
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Text {
                            text: artist
                            font.pixelSize: 12
                            color: Theme.textSecondary
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    Text {
                        text: album
                        Layout.preferredWidth: Theme.searchAlbumWidth
                        font.pixelSize: 12
                        color: Theme.textSecondary
                        elide: Text.ElideRight
                    }

                    // 喜欢
                    ToolButton {
                        Layout.preferredWidth: Theme.searchLikeWidth
                        implicitWidth: 32
                        implicitHeight: 32
                        z: 2
                        hoverEnabled: true
                        onClicked: root.likeClicked(index)
                        background: Rectangle {
                            radius: 16
                            color: parent.hovered ? Theme.bgHover : "transparent"
                        }
                        contentItem: AppIcon {
                            name: rowItem.isLiked ? Icons.heartFilled : Icons.heart
                            size: 16
                            color: rowItem.isLiked ? Theme.accent : Theme.textTertiary
                        }
                    }

                    Text {
                        text: duration
                        Layout.preferredWidth: Theme.searchDurationWidth
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: 12
                        color: Theme.textTertiary
                    }
                }

                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    // 排除「喜欢 + 时长」列，避免拦截爱心按钮点击
                    anchors.rightMargin: Theme.searchLikeWidth + Theme.searchDurationWidth + 12
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: function(mouse) {
                        if (mouse.button === Qt.RightButton) {
                            root.contextRow = index
                            rowContextMenu.popup()
                            return
                        }
                        if (root.batchMode) {
                            root.toggleBatchRow(index)
                            return
                        }
                        root.rowSelected(index)
                    }
                    onDoubleClicked: root.rowActivated(index)
                }
            }
        }

    Menu {
        id: rowContextMenu

        MenuItem {
            text: qsTr("播放")
            onTriggered: root.rowActivated(root.contextRow)
        }

        MenuItem {
            height: root.showRemoveFromPlaylist ? implicitHeight : 0
            visible: root.showRemoveFromPlaylist
            text: root.removeFromPlaylistLabel
            onTriggered: root.removeFromPlaylist(root.contextRow)
        }

        Menu {
            title: qsTr("加入歌单")

            Repeater {
                model: app.sidebarPlaylists
                delegate: MenuItem {
                    required property var modelData
                    text: modelData.name + (modelData.trackCount > 0
                           ? (" (" + modelData.trackCount + ")") : "")
                    onTriggered: root.addToPlaylist(root.contextRow, modelData.id)
                }
            }
        }
    }
}
