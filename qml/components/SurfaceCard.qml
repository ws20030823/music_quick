// SurfaceCard.qml — 半透明白蒙层卡片，内容保持实色
import QtQuick
import MusicQuick

Item {
    id: root

    property real cardShellAlpha: 0.20

    default property alias content: contentHost.data

    Rectangle {
        id: shadowPlate
        anchors.fill: cardBody
        anchors.topMargin: Theme.cardShadowOffsetY
        radius: Theme.cardShellRadius
        color: "#40000000"
        opacity: Theme.reduceMotion ? 0 : 0.12
    }

    Rectangle {
        id: cardBody
        anchors.fill: parent
        radius: Theme.cardShellRadius
        color: Theme.cardShellTint(cardShellAlpha)
        border.color: Theme.cardShellBorder(cardShellAlpha)
        border.width: 1
    }

    Item {
        id: contentHost
        anchors.fill: parent
        z: 1
    }
}
