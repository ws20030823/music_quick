// SongList.qml — 歌曲表格列表（表头 + ListView + 点击播放）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Rectangle {
    id: root
    color: Theme.bgCard
    radius: 8
    border.color: Theme.border

    property alias model: listView.model
    signal rowActivated(int row)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 1
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: Theme.bgBase
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Text { text: "#"; width: 28; color: Theme.textTertiary; font.pixelSize: 12 }
                Item { width: Theme.listCoverSize + 8 }
                Text { text: qsTr("标题"); Layout.fillWidth: true; color: Theme.textTertiary; font.pixelSize: 12 }
                Text { text: qsTr("专辑"); Layout.preferredWidth: 140; color: Theme.textTertiary; font.pixelSize: 12 }
                Text { text: qsTr("时长"); width: 48; horizontalAlignment: Text.AlignRight; color: Theme.textTertiary; font.pixelSize: 12 }
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 0

            delegate: Rectangle {
                required property int index
                required property string title
                required property string artist
                required property string album
                required property string duration
                required property bool hasCover
                required property var cover
                required property bool isPlaying

                width: ListView.view.width
                height: 52
                color: isPlaying ? Theme.accentSoft : (mouseArea.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 8

                    Item {
                        width: 28
                        height: 28
                        Text {
                            anchors.centerIn: parent
                            visible: !isPlaying
                            text: String(index + 1)
                            color: Theme.textTertiary
                            font.pixelSize: 12
                        }
                        AppIcon {
                            anchors.centerIn: parent
                            visible: isPlaying
                            name: Icons.play
                            size: 14
                            color: Theme.accent
                        }
                    }

                    Rectangle {
                        width: Theme.listCoverSize
                        height: Theme.listCoverSize
                        radius: 4
                        color: Theme.bgSidebar
                        clip: true

                        Image {
                            anchors.fill: parent
                            source: hasCover ? cover : ""
                            fillMode: Image.PreserveAspectCrop
                            visible: hasCover
                        }
                        AppIcon {
                            anchors.centerIn: parent
                            visible: !hasCover
                            name: Icons.play
                            size: 18
                            color: Theme.textTertiary
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: title
                            font.pixelSize: 14
                            color: isPlaying ? Theme.accent : Theme.textPrimary
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
                        Layout.preferredWidth: 140
                        font.pixelSize: 12
                        color: Theme.textSecondary
                        elide: Text.ElideRight
                    }

                    Text {
                        text: duration
                        width: 48
                        horizontalAlignment: Text.AlignRight
                        font.pixelSize: 12
                        color: Theme.textTertiary
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: root.rowActivated(index)
                }
            }
        }
    }
}
