// FeaturedPlaylistCard.qml — 精选歌单卡片（1:1 封面 + 悬停播放）
import QtQuick
import QtQuick.Controls
import MusicQuick

Item {
    id: root

    property string title: ""
    property string subtitle: ""
    property string coverUrl: ""
    property int cardWidth: 180

    signal clicked()

    implicitWidth: cardWidth
    implicitHeight: cardWidth + 52
    width: cardWidth
    height: cardWidth + 52
    clip: false

    readonly property bool hovered: cardMouse.containsMouse

    // 视觉层位移，不改变 GridLayout 分配的几何位置
    Item {
        id: visualHost
        width: root.width
        height: root.height

        transform: Translate {
            id: hoverLift
            y: root.hovered ? -Theme.cardHoverLift : 0

            Behavior on y {
                enabled: !Theme.reduceMotion
                NumberAnimation {
                    duration: Theme.cardCoverHoverScaleMs
                    easing.type: Easing.OutCubic
                }
            }
        }

        // 封面阴影
        Rectangle {
            x: coverBox.x + 2
            y: coverBox.y + 6
            width: coverBox.width
            height: coverBox.height
            radius: Theme.cardCoverRadius
            color: Theme.cardShadow
        }

        Item {
            id: coverBox
            width: root.cardWidth
            height: root.cardWidth

            Rectangle {
                id: coverClip
                anchors.fill: parent
                radius: Theme.cardCoverRadius
                clip: true
                color: Theme.bgCard
                border.color: root.hovered ? Theme.accentSoft : Theme.border
                border.width: 1

                Image {
                    id: coverImage
                    anchors.fill: parent
                    source: root.coverUrl
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    asynchronous: true
                    scale: root.hovered ? 1.06 : 1.0
                    transformOrigin: Item.Center

                    Behavior on scale {
                        enabled: !Theme.reduceMotion
                        NumberAnimation {
                            duration: Theme.cardCoverHoverScaleMs
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: "#33000000"
                    opacity: root.hovered ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 50
                    height: 50
                    radius: 25
                    color: "#E6FFFFFF"
                    border.color: Theme.borderStrong
                    border.width: 1
                    opacity: root.hovered ? 1 : 0
                    scale: root.hovered ? 1 : 0.85

                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }
                    Behavior on scale {
                        enabled: !Theme.reduceMotion
                        NumberAnimation { duration: 200; easing.type: Easing.OutBack }
                    }

                    AppIcon {
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: 1
                        name: Icons.play
                        size: 18
                        color: Theme.textPrimary
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    visible: coverImage.status === Image.Error
                             || (coverImage.status === Image.Ready && coverImage.sourceSize.width === 0)
                    color: Theme.accentSoft
                    Text {
                        anchors.centerIn: parent
                        text: root.title.length > 0 ? root.title.charAt(0) : "?"
                        font.pixelSize: 36
                        font.bold: true
                        color: Theme.accent
                    }
                }
            }
        }

        Column {
            anchors.top: coverBox.bottom
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 4

            Text {
                width: parent.width
                text: root.title
                font.pixelSize: 15
                font.weight: Font.DemiBold
                color: Theme.textPrimary
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                width: parent.width
                text: root.subtitle
                font.pixelSize: 13
                color: Theme.textSecondary
                elide: Text.ElideRight
                maximumLineCount: 1
            }
        }
    }

    MouseArea {
        id: cardMouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
