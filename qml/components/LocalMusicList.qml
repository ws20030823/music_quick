// LocalMusicList.qml — 网易云风格本地音乐表格（单击选中 / 双击播放 / ♥ 收藏）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    property alias model: listView.model
    signal rowSelected(int row)
    signal rowActivated(int row)
    signal likeClicked(int row)

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.searchTableHeaderHeight
        color: "transparent"

        RowLayout {
            anchors.fill: parent
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
            Text {
                text: qsTr("大小")
                Layout.preferredWidth: Theme.searchFileSizeWidth
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
            required property string fileSize
            required property bool isPlaying
            required property bool isSelected
            required property bool isLiked

            width: ListView.view.width
            height: Theme.searchRowHeight
            color: isPlaying || isSelected ? Theme.accentSoft
                 : (rowMouse.containsMouse ? Theme.bgHover : "transparent")

            Behavior on color { ColorAnimation { duration: 150 } }

            RowLayout {
                anchors.fill: parent
                spacing: 12

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
                        color: isSelected ? Theme.accent : Theme.textSecondary
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

                ToolButton {
                    Layout.preferredWidth: Theme.searchLikeWidth
                    implicitWidth: 32
                    implicitHeight: 32
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

                Text {
                    text: fileSize
                    Layout.preferredWidth: Theme.searchFileSizeWidth
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: 12
                    color: Theme.textTertiary
                }
            }

            MouseArea {
                id: rowMouse
                anchors.fill: parent
                anchors.rightMargin: Theme.searchLikeWidth + 8
                hoverEnabled: true
                onClicked: root.rowSelected(index)
                onDoubleClicked: root.rowActivated(index)
            }
        }
    }
}
