// SearchPage.qml — 搜索页占位
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 8

        Text {
            text: qsTr("搜索")
            font.pixelSize: 22
            font.bold: true
            color: Theme.textPrimary
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("搜索功能尚未实现（与 Widgets 版一致，仅占位）")
            font.pixelSize: 14
            color: Theme.textSecondary
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
