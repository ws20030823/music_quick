// QueueDialog.qml — 播放队列弹窗
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Popup {
    id: root
    modal: true
    anchors.centerIn: Overlay.overlay
    width: Math.min(480, Overlay.overlay ? Overlay.overlay.width - 48 : 480)
    height: Math.min(520, Overlay.overlay ? Overlay.overlay.height - 96 : 520)
    padding: 0

    onAboutToShow: queueList.model = app.queueItems()
    onClosed: app.queueVisible = false

    background: Rectangle {
        color: Theme.bgCard
        radius: 12
        border.color: Theme.border
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: qsTr("播放队列")
                font.pixelSize: 18
                font.bold: true
                color: Theme.textPrimary
                Layout.fillWidth: true
            }
            ToolButton {
                text: "✕"
                onClicked: root.close()
            }
        }

        ListView {
            id: queueList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 2

            delegate: Rectangle {
                required property int index
                required property var modelData

                width: queueList.width
                height: 48
                radius: 6
                color: modelData.isPlaying ? Theme.accentSoft
                     : (mouseArea.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Text {
                        text: modelData.isPlaying ? "▶" : String(modelData.row + 1)
                        width: 24
                        color: modelData.isPlaying ? Theme.accent : Theme.textTertiary
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: modelData.title
                            font.pixelSize: 14
                            color: Theme.textPrimary
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Text {
                            text: modelData.artist
                            font.pixelSize: 12
                            color: Theme.textSecondary
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    Text {
                        text: modelData.duration
                        font.pixelSize: 12
                        color: Theme.textTertiary
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        app.playQueueRow(modelData.row)
                        root.close()
                    }
                }
            }
        }
    }
}
