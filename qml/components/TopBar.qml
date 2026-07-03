// TopBar.qml — 网易云风格顶栏：Logo + 竖线 + 后退/搜索/窗口控制
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Rectangle {
    id: root
    color: Theme.topBarBg
    implicitHeight: Theme.topBarHeight

    required property Window window

    property alias searchField: searchInput
    property point dragStart

    signal searchSubmitted(string keyword)

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 0
        spacing: 0

        // ── Logo + 品牌名（紧凑宽度，竖线紧跟文字右侧）──
        Item {
            id: brandArea
            Layout.preferredHeight: Theme.topBarHeight
            Layout.preferredWidth: brandRow.implicitWidth
            Layout.alignment: Qt.AlignVCenter

            Row {
                id: brandRow
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Image {
                    width: 28
                    height: 28
                    source: Icons.url(Icons.logo)
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    antialiasing: true
                }
                Text {
                    id: brandText
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Music Quick")
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textPrimary
                }
            }

            MouseArea {
                anchors.fill: parent
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
        }

        Item { Layout.preferredWidth: 12 }

        // ── 竖线（品牌名右侧）──
        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 20
            Layout.alignment: Qt.AlignVCenter
            color: Theme.borderStrong
        }

        Item { Layout.preferredWidth: 10 }

        // ── 后退 + 搜索 + 语音 + 窗口控制 ──
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            ToolButton {
                implicitWidth: 32
                implicitHeight: 32
                enabled: false
                opacity: 0.4
                Layout.alignment: Qt.AlignVCenter
                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? Theme.bgHover : Theme.bgCard
                    border.color: Theme.borderStrong
                }
                contentItem: AppIcon {
                    name: Icons.chevronLeft
                    size: 16
                    color: Theme.textSecondary
                    anchors.centerIn: parent
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 480
                Layout.preferredHeight: 34
                Layout.alignment: Qt.AlignVCenter
                radius: 17
                color: Theme.bgCard
                border.color: searchInput.activeFocus ? Theme.accent : Theme.borderStrong
                z: 2

                Behavior on border.color { ColorAnimation { duration: 200 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 10
                    spacing: 8

                    AppIcon {
                        name: Icons.search
                        size: 15
                        color: Theme.textTertiary
                    }

                    TextField {
                        id: searchInput
                        Layout.fillWidth: true
                        placeholderText: app.searchKeyword.length > 0
                                             ? app.searchKeyword
                                             : qsTr("搜索音乐、歌手")
                        font.pixelSize: 13
                        color: Theme.textPrimary
                        placeholderTextColor: Theme.textTertiary
                        selectByMouse: true
                        background: Item {}
                        onAccepted: root.searchSubmitted(text.trim())
                    }
                }
            }

            ToolButton {
                implicitWidth: 32
                implicitHeight: 32
                enabled: false
                opacity: 0.4
                Layout.alignment: Qt.AlignVCenter
                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? Theme.bgHover : Theme.bgCard
                    border.color: Theme.borderStrong
                }
                contentItem: AppIcon {
                    name: Icons.mic
                    size: 16
                    color: Theme.textSecondary
                    anchors.centerIn: parent
                }
            }

            // 空白区可拖拽，不遮挡搜索框
            Item {
                id: dragSpacer
                Layout.fillWidth: true
                Layout.fillHeight: true

                MouseArea {
                    anchors.fill: parent
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
            }

            Rectangle {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 20
                Layout.alignment: Qt.AlignVCenter
                color: Theme.borderStrong
            }

            WindowControls {
                id: windowControls
                Layout.alignment: Qt.AlignVCenter
                window: root.window
            }
        }
    }
}
