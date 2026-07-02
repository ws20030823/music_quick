// SearchPage.qml — 搜索页占位
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Theme.pagePadding * 2, 480)
        spacing: 16

        Text {
            text: qsTr("搜索")
            font.pixelSize: 22
            font.bold: true
            color: Theme.textPrimary
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: 22
            color: Theme.bgCard
            border.color: Theme.border

            Text {
                anchors.centerIn: parent
                text: qsTr("在顶部搜索框输入关键词（功能即将推出）")
                font.pixelSize: 13
                color: Theme.textTertiary
            }
        }

        Text {
            text: qsTr("搜索功能尚未实现，与 Widgets 版一致，仅占位。")
            font.pixelSize: 14
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }
}
