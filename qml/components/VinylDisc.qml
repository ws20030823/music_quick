// VinylDisc.qml — 黑膠唱片 + 唱臂（SVG 圖層 + 旋轉動畫）
import QtQuick
import QtQuick.Effects
import MusicQuick

Item {
    id: root

    property bool playing: false
    property string coverUrl: ""
    property bool hasCover: false
    property int discSize: Theme.npVinylSize

    implicitWidth: discSize + tonearm.width * 0.35
    implicitHeight: discSize + 24
    width: implicitWidth
    height: implicitHeight

    readonly property real labelDiameter: discSize * (170 / 300)

    Item {
        id: discStage
        width: root.discSize
        height: root.discSize
        anchors.centerIn: parent

        // 唱片下方陰影
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.92
            height: width
            radius: width / 2
            color: "#14000000"
            y: 10
        }

        // 唱片組（黑膠環 + 封面同步旋轉）
        Item {
            id: discGroup
            anchors.fill: parent

            RotationAnimation on rotation {
                running: root.playing
                from: 0
                to: 360
                duration: Theme.npVinylRotationMs
                loops: Animation.Infinite
            }

            Image {
                anchors.fill: parent
                source: "qrc:/nowplaying/disc_ring.svg"
                smooth: true
                antialiasing: true
            }

            Item {
                id: albumWrapper
                width: root.labelDiameter
                height: width
                anchors.centerIn: parent

                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: Theme.accent
                    visible: !root.hasCover
                }

                Image {
                    id: albumImage
                    anchors.fill: parent
                    source: root.coverUrl
                    cache: false
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    asynchronous: true
                    visible: false
                }

                Item {
                    id: maskItem
                    anchors.fill: parent
                    layer.enabled: true
                    visible: false

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: "white"
                    }
                }

                MultiEffect {
                    anchors.fill: parent
                    source: albumImage
                    maskEnabled: root.hasCover
                    maskSource: maskItem
                    visible: root.hasCover
                }

                AppIcon {
                    anchors.centerIn: parent
                    visible: !root.hasCover
                    name: Icons.play
                    size: Math.round(root.labelDiameter * 0.22)
                    color: "#FFFFFF"
                }
            }

            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "#1a1a1e"
                border.color: "#44444a"
                border.width: 1
                anchors.centerIn: parent
            }
        }

        // 唱臂（不隨唱片旋轉）
        Image {
            id: tonearm
            source: "qrc:/nowplaying/tonearm.svg"
            width: root.discSize * 0.28
            height: width * 2
            x: discStage.width * 0.62
            y: -4
            transformOrigin: Item.Top
            rotation: root.playing ? Theme.npTonearmPlayingDeg : Theme.npTonearmRestDeg
            smooth: true
            antialiasing: true
            z: 2

            Behavior on rotation {
                NumberAnimation {
                    duration: Theme.npTonearmAnimMs
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}
