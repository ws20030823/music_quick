// HomePage.qml — 推荐首页（卡片网格占位）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    signal openLocalMusic()

    readonly property var placeholderPlaylists: [
        { title: qsTr("惬意午后"), subtitle: qsTr("轻音乐精选") },
        { title: qsTr("专注工作"), subtitle: qsTr("纯音乐") },
        { title: qsTr("通勤路上"), subtitle: qsTr("流行热歌") },
        { title: qsTr("深夜情绪"), subtitle: qsTr("慢节奏") },
        { title: qsTr("运动节拍"), subtitle: qsTr("高能律动") },
        { title: qsTr("学习背景"), subtitle: qsTr("Lo-Fi") },
        { title: qsTr("周末宅家"), subtitle: qsTr("华语精选") },
        { title: qsTr("旅途风景"), subtitle: qsTr("独立音乐") }
    ]

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: root.width
            spacing: Theme.sectionGap

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.topMargin: Theme.pagePadding

                ColumnLayout {
                    spacing: 4
                    Text {
                        text: qsTr("推荐")
                        font.pixelSize: 22
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    Text {
                        text: qsTr("功能占位，后续可接入推荐与歌单")
                        font.pixelSize: 13
                        color: Theme.textSecondary
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: qsTr("去本地音乐")
                    onClicked: root.openLocalMusic()
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.bgHover : Theme.bgCard
                        border.color: Theme.border
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.textPrimary
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            Text {
                Layout.leftMargin: Theme.pagePadding
                text: qsTr("精选歌单")
                font.pixelSize: 16
                font.bold: true
                color: Theme.textPrimary
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.bottomMargin: Theme.pagePadding
                columns: Math.max(2, Math.floor((root.width - Theme.pagePadding * 2) / 200))
                columnSpacing: Theme.sectionGap
                rowSpacing: Theme.sectionGap

                Repeater {
                    model: root.placeholderPlaylists

                    PlaylistCard {
                        required property var modelData
                        required property int index
                        title: modelData.title
                        subtitle: modelData.subtitle
                        accentIndex: index
                        onClicked: root.openLocalMusic()
                    }
                }
            }
        }
    }

    component PlaylistCard: Rectangle {
        id: card
        property string title: ""
        property string subtitle: ""
        property int accentIndex: 0

        signal clicked()

        Layout.preferredWidth: 180
        Layout.preferredHeight: 220
        radius: Theme.radiusLg
        color: Theme.bgCard
        border.color: cardMouse.containsMouse ? Theme.accentSoft : Theme.border

        readonly property color coverTint: [
            "#5B8DEF", "#7C6CF0", "#4DB6AC", "#FFB74D",
            "#F06292", "#64B5F6", "#81C784", "#9575CD"
        ][accentIndex % 8]

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 140
                radius: Theme.radiusMd
                color: card.coverTint

                Text {
                    anchors.centerIn: parent
                    text: card.title.charAt(0)
                    font.pixelSize: 36
                    font.bold: true
                    color: "#FFFFFFCC"
                }
            }

            Text {
                text: card.title
                font.pixelSize: 14
                font.bold: true
                color: Theme.textPrimary
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Text {
                text: card.subtitle
                font.pixelSize: 12
                color: Theme.textSecondary
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        MouseArea {
            id: cardMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: card.clicked()
        }
    }
}
