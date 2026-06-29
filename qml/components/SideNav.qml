// SideNav.qml — 左侧导航栏（三页切换 + 导入音乐 + 状态栏）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Rectangle {
    id: root
    color: Theme.bgSidebar

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.border
    }

    property int currentPage: 0
    signal navigate(int page)
    signal importClicked()

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 4

        Row {
            spacing: 8
            Rectangle {
                width: 32
                height: 32
                radius: 8
                color: Theme.accent
                AppIcon {
                    anchors.centerIn: parent
                    name: Icons.play
                    size: 18
                    color: "white"
                }
            }
            Text {
                text: qsTr("Music Quick")
                font.pixelSize: 16
                font.bold: true
                color: Theme.textPrimary
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            text: qsTr("导航")
            font.pixelSize: 11
            color: Theme.textTertiary
            topPadding: 12
        }

        NavButton { label: qsTr("首页"); page: 0 }
        NavButton { label: qsTr("本地音乐"); page: 1 }
        NavButton { label: qsTr("搜索"); page: 2 }

        Item { height: 16; width: 1 }

        Button {
            width: parent.width
            text: qsTr("导入音乐")
            onClicked: root.importClicked()
            background: Rectangle {
                radius: 8
                color: parent.down ? Theme.accent : (parent.hovered ? "#1A0078D4" : Theme.accentSoft)
            }
            contentItem: Text {
                text: parent.text
                color: Theme.accent
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 14
            }
        }

        Item { width: 1; height: 24 }

        Text {
            width: parent.width
            text: app.statusText
            font.pixelSize: 11
            color: Theme.textTertiary
            wrapMode: Text.WordWrap
        }
    }

    component NavButton: Button {
        property string label
        property int page
        width: parent.width
        text: label
        flat: true
        checkable: true
        checked: root.currentPage === page
        onClicked: root.navigate(page)
        background: Item {
            id: navBg
            implicitHeight: 40
            implicitWidth: parent.width

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: navBg.parent.checked ? Theme.accentSoft
                     : (navBg.parent.hovered ? Theme.bgHover : "transparent")
            }
            Rectangle {
                width: navBg.parent.checked ? 4 : 0
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: Theme.accent
                visible: navBg.parent.checked
            }
        }
        contentItem: Text {
            text: parent.text
            color: parent.checked ? Theme.accent : Theme.textSecondary
            font.pixelSize: 14
            font.weight: parent.checked ? Font.DemiBold : Font.Normal
            horizontalAlignment: Text.AlignLeft
            leftPadding: parent.checked ? 8 : 12
        }
    }
}
