// TopBar.qml — 全宽顶栏：Logo、搜索、窗口控制
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Rectangle {
    id: root
    color: Theme.bgCard
    implicitHeight: Theme.topBarHeight

    required property Window window

    property alias searchField: searchInput
    property point dragStart

    signal searchSubmitted(string keyword)

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.border
    }

    MouseArea {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Theme.sidebarWidth
        onPressed: function(mouse) {
            root.dragStart = Qt.point(mouse.x, mouse.y)
        }
        onPositionChanged: function(mouse) {
            if (pressed && root.window) {
                root.window.x += mouse.x - root.dragStart.x
                root.window.y += mouse.y - root.dragStart.y
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧 Logo（与侧栏同宽对齐）
        RowLayout {
            Layout.preferredWidth: Theme.sidebarWidth
            Layout.minimumWidth: Theme.sidebarWidth
            Layout.fillHeight: true
            Layout.leftMargin: 16
            spacing: 8

            Image {
                Layout.preferredWidth: Theme.logoSize
                Layout.preferredHeight: Theme.logoSize
                Layout.maximumWidth: Theme.logoSize
                Layout.maximumHeight: Theme.logoSize
                source: Icons.url(Icons.logo)
                fillMode: Image.PreserveAspectFit
                smooth: true
                antialiasing: true
            }
            Text {
                text: qsTr("Music Quick")
                font.pixelSize: 15
                font.bold: true
                color: Theme.textPrimary
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 4
            spacing: 12

            Row {
                spacing: 4
                Layout.alignment: Qt.AlignVCenter

                ToolButton {
                    implicitWidth: 32
                    implicitHeight: 32
                    enabled: false
                    opacity: 0.35
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.bgHover : "transparent"
                    }
                    contentItem: Text {
                        text: "◀"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                ToolButton {
                    implicitWidth: 32
                    implicitHeight: 32
                    enabled: false
                    opacity: 0.35
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.bgHover : "transparent"
                    }
                    contentItem: Text {
                        text: "▶"
                        font.pixelSize: 12
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 420
                Layout.preferredHeight: 36
                radius: 18
                color: Theme.bgBase
                border.color: searchInput.activeFocus ? Theme.accentSoft : Theme.border

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 8
                    spacing: 8

                    Text {
                        text: qsTr("搜")
                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.textTertiary
                    }

                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        placeholderText: qsTr("搜索音乐、歌手")
                        font.pixelSize: 13
                        color: Theme.textPrimary
                        background: Item {}
                        onAccepted: {
                            app.currentPage = 2
                            root.searchSubmitted(text)
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        WindowControls {
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            window: root.window
        }
    }
}
