// IconTransportButton.qml — 播放栏传输控制按钮（默认样式）
import QtQuick
import QtQuick.Controls
import MusicQuick

ToolButton {
    id: ctrl

    property string iconName: ""
    property int iconSize: 20
    property bool interactive: true
    property bool suppressNowPlayingOpen: true
    property color iconColor: interactive ? Theme.textPrimary : Theme.iconMuted

    implicitWidth: 36
    implicitHeight: 36
    enabled: true

    background: Rectangle {
        radius: ctrl.implicitWidth / 2
        color: ctrl.down ? Theme.accentSoft
             : (ctrl.hovered && ctrl.interactive ? Theme.bgHover : "transparent")
    }

    contentItem: Item {
        implicitWidth: ctrl.iconSize
        implicitHeight: ctrl.iconSize
        AppIcon {
            anchors.centerIn: parent
            width: ctrl.iconSize
            height: ctrl.iconSize
            name: ctrl.iconName
            color: ctrl.iconColor
        }
    }
}
