// SettingsCard.qml — 设置页卡片容器（半透明蒙层，内容实色）
import QtQuick
import QtQuick.Layouts
import MusicQuick

Item {
    id: card

    property string title: ""
    property string subtitle: ""
    property real cardShellAlpha: app.uiCardShellOpacity
    default property alias content: cardContent.data

    Layout.fillWidth: true
    implicitHeight: cardLayout.implicitHeight + 40

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusLg
        color: Theme.cardShellTint(card.cardShellAlpha)
        border.color: Theme.cardShellBorder(card.cardShellAlpha)
        border.width: 1
    }

    ColumnLayout {
        id: cardLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 16
        z: 1

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: card.title.length > 0

            Text {
                text: card.title
                font.pixelSize: 15
                font.bold: true
                color: Theme.textPrimary
            }

            Text {
                visible: card.subtitle.length > 0
                text: card.subtitle
                font.pixelSize: 12
                color: Theme.textTertiary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                lineHeight: 1.35
            }
        }

        ColumnLayout {
            id: cardContent
            Layout.fillWidth: true
            spacing: 12
        }
    }
}
