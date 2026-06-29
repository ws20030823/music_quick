// =============================================================================
// Main.qml — 应用主窗口
// 布局：SideNav + StackLayout(三页) + PlaybackBar；FileDialog 导入；QueueDialog 队列
// =============================================================================
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import MusicQuick

ApplicationWindow {
    id: root
    width: 1100
    height: 720
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("Music Quick")
    color: Theme.bgBase

    FileDialog {
        id: importDialog
        title: qsTr("选择音乐文件")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("音频文件 (*.mp3 *.wav *.flac *.aac)"), qsTr("所有文件 (*.*)")]
        onAccepted: app.importFiles(selectedFiles)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        SideNav {
            Layout.preferredWidth: Theme.sidebarWidth
            Layout.minimumWidth: Theme.sidebarWidth
            Layout.fillHeight: true
            currentPage: app.currentPage
            onNavigate: function(page) { app.currentPage = page }
            onImportClicked: importDialog.open()
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.bgBase

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: app.currentPage

                    HomePage { }
                    LocalMusicPage {
                        trackModel: app.trackModel
                        onImportClicked: importDialog.open()
                    }
                    SearchPage { }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Theme.border
                }

                PlaybackBar {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.playbackBarHeight
                    Layout.minimumHeight: Theme.playbackBarHeight
                }
            }
        }
    }

    QueueDialog {
        id: queueDialog
    }

    Connections {
        target: app
        function onQueueVisibleChanged() {
            if (app.queueVisible) {
                queueDialog.open()
            } else {
                queueDialog.close()
            }
        }
    }
}
