// AppIcon.qml — 统一 SVG 图标渲染（size + color）
import QtQuick
import QtQuick.Effects
import MusicQuick

Item {
    id: root

    property string name: ""
    property int size: 24
    property color color: Theme.textPrimary

    width: size
    height: size
    implicitWidth: size
    implicitHeight: size

    Image {
        id: image
        anchors.fill: parent
        source: root.name !== "" ? Icons.url(root.name) : ""
        sourceSize: Qt.size(root.size * 2, root.size * 2)
        fillMode: Image.PreserveAspectFit
        smooth: true
        antialiasing: true
        visible: false
    }

    MultiEffect {
        anchors.fill: parent
        source: image
        colorization: 1.0
        colorizationColor: root.color
    }
}
