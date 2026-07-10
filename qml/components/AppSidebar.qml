// AppSidebar.qml — 左侧容器：Logo + 品牌名 + 导航菜单（网易云布局）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Rectangle {
    id: root
    color: "transparent"
    implicitWidth: Theme.sidebarWidth

    required property Window window
    property int currentPage: 0

    signal navigate(int page)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.topBarHeight

            Row {
                id: brandRow
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Image {
                    width: Theme.logoSize
                    height: Theme.logoSize
                    source: Icons.url(Icons.logo)
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    antialiasing: true
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("WingSound")
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textPrimary
                }
            }

            MouseArea {
                anchors.fill: parent
                onPressed: function(mouse) {
                    if (mouse.button === Qt.LeftButton && root.window)
                        root.window.startSystemMove()
                }
            }
        }

        SideNav {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentPage: root.currentPage
            onNavigate: function(page) { root.navigate(page) }
        }
    }
}
