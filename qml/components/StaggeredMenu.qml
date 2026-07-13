// StaggeredMenu.qml — 交错滑入侧栏菜单（移植自 React Bits StaggeredMenu）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    property var items: []
    property var colors: []
    property string position: "left"
    property bool opened: false
    property int currentIndex: 0
    property color accentColor: Theme.accent
    property bool displayItemNumbering: true
    property bool showCloseButton: true
    property real cardShellAlpha: 0.20

    signal itemSelected(int index)
    signal menuOpened()
    signal menuClosed()
    signal closeRequested()

    readonly property real slideWidth: Math.max(width, Theme.sidebarWidth)
    readonly property bool fromLeft: position === "left"
    readonly property real hideX: fromLeft ? -slideWidth : slideWidth

    property bool ready: false

    function snapHidden() {
        pre0Shift.x = hideX
        pre1Shift.x = hideX
        panelShift.x = hideX
    }

    function playOpenAnimation() {
        openSlideAnim.stop()
        closeSlideAnim.stop()
        if (Theme.reduceMotion) {
            pre0Shift.x = 0
            pre1Shift.x = 0
            panelShift.x = 0
            return
        }
        snapHidden()
        openSlideAnim.start()
    }

    function playCloseAnimation() {
        openSlideAnim.stop()
        closeSlideAnim.stop()
        if (Theme.reduceMotion) {
            snapHidden()
            return
        }
        closeSlideAnim.start()
    }

    onOpenedChanged: {
        if (!ready)
            return
        if (opened) {
            playOpenAnimation()
            menuOpened()
        } else {
            playCloseAnimation()
            menuClosed()
        }
    }

    onWidthChanged: {
        if (!opened)
            snapHidden()
    }

    Component.onCompleted: {
        snapHidden()
        ready = true
    }

    ParallelAnimation {
        id: openSlideAnim
        NumberAnimation {
            target: pre0Shift
            property: "x"
            to: 0
            duration: 460
            easing.type: Easing.OutCubic
        }
        SequentialAnimation {
            PauseAnimation { duration: 60 }
            NumberAnimation {
                target: pre1Shift
                property: "x"
                to: 0
                duration: 480
                easing.type: Easing.OutCubic
            }
        }
        SequentialAnimation {
            PauseAnimation { duration: 110 }
            NumberAnimation {
                target: panelShift
                property: "x"
                to: 0
                duration: 620
                easing.type: Easing.OutCubic
            }
        }
    }

    ParallelAnimation {
        id: closeSlideAnim
        NumberAnimation {
            target: panelShift
            property: "x"
            to: hideX
            duration: 360
            easing.type: Easing.InCubic
        }
        SequentialAnimation {
            PauseAnimation { duration: 40 }
            NumberAnimation {
                target: pre1Shift
                property: "x"
                to: hideX
                duration: 320
                easing.type: Easing.InCubic
            }
        }
        SequentialAnimation {
            PauseAnimation { duration: 80 }
            NumberAnimation {
                target: pre0Shift
                property: "x"
                to: hideX
                duration: 300
                easing.type: Easing.InCubic
            }
        }
    }

    Rectangle {
        id: preLayer0
        y: 0
        height: parent.height
        width: root.slideWidth
        x: root.fromLeft ? 0 : parent.width - width
        radius: Theme.cardShellRadius
        color: root.colors.length > 0
               ? root.colors[0]
               : Theme.cardShellTint(root.cardShellAlpha * 0.55)
        border.color: Theme.cardShellBorder(root.cardShellAlpha * 0.55)
        border.width: 1
        z: 1
        transform: Translate { id: pre0Shift; x: root.hideX }
    }

    Rectangle {
        id: preLayer1
        y: 0
        height: parent.height
        width: root.slideWidth
        x: root.fromLeft ? 0 : parent.width - width
        radius: Theme.cardShellRadius
        color: root.colors.length > 0
               ? root.colors[1]
               : Theme.cardShellTint(root.cardShellAlpha * 0.82)
        border.color: Theme.cardShellBorder(root.cardShellAlpha * 0.82)
        border.width: 1
        z: 2
        transform: Translate { id: pre1Shift; x: root.hideX }
    }

    Item {
        id: menuPanel
        y: 0
        height: parent.height
        width: root.slideWidth
        x: root.fromLeft ? 0 : parent.width - width
        z: 3
        transform: Translate { id: panelShift; x: root.hideX }

        Rectangle {
            id: menuShadow
            anchors.fill: menuCardBody
            anchors.topMargin: Theme.cardShadowOffsetY
            radius: Theme.cardShellRadius
            color: "#40000000"
            opacity: Theme.reduceMotion ? 0 : 0.12
            visible: !Theme.reduceMotion
        }

        Rectangle {
            id: menuCardBody
            anchors.fill: parent
            radius: Theme.cardShellRadius
            color: Theme.cardShellTint(root.cardShellAlpha)
            border.color: Theme.cardShellBorder(root.cardShellAlpha)
            border.width: 1
        }

        Column {
            anchors.fill: parent
            anchors.topMargin: 16
            anchors.leftMargin: 20
            anchors.rightMargin: 12
            anchors.bottomMargin: 20
            spacing: 4

            RowLayout {
                width: parent.width
                spacing: 8

                Text {
                    text: qsTr("设置")
                    font.pixelSize: 18
                    font.bold: true
                    color: Theme.textPrimary
                    Layout.fillWidth: true
                }

                ToolButton {
                    visible: root.showCloseButton
                    implicitWidth: 32
                    implicitHeight: 32
                    ToolTip.text: qsTr("关闭")
                    onClicked: root.closeRequested()
                    background: Rectangle {
                        radius: 8
                        color: parent.hovered ? Theme.bgHover : "transparent"
                    }
                    contentItem: Text {
                        text: "\u00D7"
                        font.pixelSize: 18
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Item { width: 1; height: 12 }

            Repeater {
                model: root.items.length

                delegate: Item {
                    id: rowHost
                    required property int index
                    width: parent.width
                    height: 52
                    opacity: itemVisible ? 1 : 0
                    property bool itemVisible: !root.opened ? false : (Theme.reduceMotion ? true : false)

                    transform: Translate {
                        y: rowHost.itemVisible ? 0 : 24
                        Behavior on y {
                            enabled: !Theme.reduceMotion
                            NumberAnimation { duration: 480; easing.type: Easing.OutCubic }
                        }
                    }

                    Timer {
                        interval: 160 + index * 80
                        running: root.opened && !Theme.reduceMotion
                        onTriggered: rowHost.itemVisible = true
                    }

                    Connections {
                        target: root
                        function onOpenedChanged() {
                            if (!root.opened)
                                rowHost.itemVisible = false
                            else if (Theme.reduceMotion)
                                rowHost.itemVisible = true
                        }
                    }

                    Behavior on opacity {
                        enabled: !Theme.reduceMotion
                        NumberAnimation { duration: 360 }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.leftMargin: 4
                        anchors.rightMargin: 8
                        radius: 8
                        color: index === root.currentIndex ? Theme.bgHover : "transparent"
                        visible: index === root.currentIndex
                    }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        spacing: 8

                        Text {
                            visible: root.displayItemNumbering
                            text: ("0" + (index + 1)).slice(-2)
                            font.pixelSize: 13
                            font.weight: Font.Medium
                            color: index === root.currentIndex ? Theme.textPrimary : Theme.textTertiary
                            width: 26
                        }

                        Text {
                            text: root.items[index].label !== undefined
                                  ? root.items[index].label : ""
                            font.pixelSize: Math.min(30, Math.max(20, root.width * 0.085))
                            font.bold: index === root.currentIndex
                            font.letterSpacing: -0.5
                            color: Theme.textPrimary
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        enabled: root.opened
                        onClicked: {
                            root.currentIndex = index
                            root.itemSelected(index)
                        }
                    }
                }
            }
        }
    }
}
