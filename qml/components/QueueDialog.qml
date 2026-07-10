// QueueDialog.qml — 播放队列弹窗
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Popup {
    id: root
    modal: true
    anchors.centerIn: Overlay.overlay
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    width: Math.min(480, Overlay.overlay ? Overlay.overlay.width - 48 : 480)
    implicitHeight: card.implicitHeight

    onAboutToShow: queueList.model = app.queueItems()
    onClosed: app.queueVisible = false

    background: Item { }

    contentItem: CardDialogSurface {
        id: card
        minWidth: Math.min(480, Overlay.overlay ? Overlay.overlay.width - 48 : 480)

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.dialogGap

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 28

                Text {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("播放队列")
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    color: Theme.dialogHeading
                }

                ToolButton {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    implicitWidth: 28
                    implicitHeight: 28
                    onClicked: root.close()
                    background: Item {}
                    contentItem: Text {
                        text: "\u00D7"
                        font.pixelSize: 20
                        color: parent.hovered ? Theme.dialogExitHover : Theme.dialogExit
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            ListView {
                id: queueList
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(420, Math.max(160, count * 50))
                clip: true
                spacing: 2

                delegate: Rectangle {
                    required property int index
                    required property var modelData

                    width: queueList.width
                    height: 48
                    radius: Theme.dialogButtonRadius
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
}
