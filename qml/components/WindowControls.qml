// WindowControls.qml — 自定义窗口最小化 / 最大化 / 关闭
import QtQuick
import QtQuick.Controls
import QtQuick.Window
import MusicQuick

Row {
    id: root
    spacing: 0

    required property Window window

    readonly property bool isMaximized: window
        && (window.visibility === Window.Maximized
            || window.visibility === Window.FullScreen)

    function toggleMaximize() {
        if (!window) {
            return
        }
        if (window.visibility === Window.Maximized || window.visibility === Window.FullScreen) {
            window.showNormal()
        } else {
            window.showMaximized()
        }
    }

    component ChromeButton: ToolButton {
        id: btn
        property string iconName: ""
        property color hoverColor: Theme.bgHover
        property color activeColor: "#E81123"
        property color activeText: "white"
        property bool danger: false

        implicitWidth: 46
        implicitHeight: Theme.topBarHeight

        background: Rectangle {
            color: btn.down ? (btn.danger ? btn.activeColor : Theme.accentSoft)
                 : (btn.hovered ? (btn.danger ? btn.activeColor : btn.hoverColor) : "transparent")
        }
        contentItem: AppIcon {
            name: btn.iconName
            size: 14
            color: btn.danger && (btn.hovered || btn.down) ? btn.activeText : Theme.textSecondary
            anchors.centerIn: parent
        }
    }

    ChromeButton {
        iconName: Icons.minimize
        onClicked: root.window.showMinimized()
    }
    ChromeButton {
        iconName: root.isMaximized ? Icons.restore : Icons.maximize
        ToolTip.visible: hovered
        ToolTip.text: root.isMaximized ? qsTr("还原") : qsTr("最大化")
        onClicked: root.toggleMaximize()
    }
    ChromeButton {
        iconName: Icons.close
        danger: true
        onClicked: root.window.close()
    }
}
