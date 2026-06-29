// HomePage.qml — 首页欢迎与引导
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12

        Text {
            text: qsTr("欢迎使用 Music Quick")
            font.pixelSize: 28
            font.bold: true
            color: Theme.textPrimary
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("Qt Quick 版音乐播放器 — 与 Widgets 版功能对齐的学习项目")
            font.pixelSize: 14
            color: Theme.textSecondary
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("点击侧栏「导入音乐」开始，或切换到「本地音乐」查看列表")
            font.pixelSize: 13
            color: Theme.textTertiary
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
        }
    }
}
