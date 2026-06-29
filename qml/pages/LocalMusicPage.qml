// LocalMusicPage.qml — 本地音乐列表页
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root
    property var trackModel
    signal importClicked()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: qsTr("本地音乐")
                font.pixelSize: 22
                font.bold: true
                color: Theme.textPrimary
            }

            Item { Layout.fillWidth: true }

            Text {
                text: qsTr("共 %1 首歌曲").arg(app.trackCount)
                font.pixelSize: 13
                color: Theme.textSecondary
            }

            Button {
                text: qsTr("导入音乐")
                onClicked: root.importClicked()
                background: Rectangle {
                    radius: 8
                    color: parent.down ? Theme.accent
                         : (parent.hovered ? "#1A0078D4" : Theme.accentSoft)
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.accent
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        SongList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.trackModel
            onRowActivated: function(row) { app.playRow(row) }
        }

        Text {
            Layout.fillWidth: true
            visible: app.trackCount === 0
            text: qsTr("暂无歌曲，请点击「导入音乐」添加本地音频文件")
            color: Theme.textTertiary
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            topPadding: 40
        }
    }
}
