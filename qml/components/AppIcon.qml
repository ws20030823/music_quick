// AppIcon.qml — 统一 SVG 图标渲染（size + color）
import QtQuick
import QtQuick.Effects
import MusicQuick

Item {
    id: root

    // Icons.qml 中的名称，如 Icons.play
    property string name: ""
    property int size: 24
    property color color: Theme.textSecondary

    implicitWidth: size
    implicitHeight: size

    Image {
        id: image
        anchors.fill: parent
        source: root.name !== "" ? Icons.url(root.name) : ""
        sourceSize: Qt.size(root.size, root.size)
        fillMode: Image.PreserveAspectFit
        visible: false
    }

    MultiEffect {
        anchors.fill: parent
        source: image
        colorization: 1.0
        colorizationColor: root.color
    }
}
