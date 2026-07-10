// SurfaceCard.qml — 实色卡片，随皮肤不透明度整体变淡
import QtQuick
import MusicQuick

Item {
    id: root

    property real cardOpacity: 1.0

    default property alias content: contentHost.data

    opacity: Theme.mapSkinOpacity(cardOpacity)

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
        color: Theme.bgCard
        border.color: Theme.cardBorder
        border.width: 1
    }

    Item {
        id: contentHost
        anchors.fill: parent
    }
}
