// Main.qml — 无边框窗口 + 侧栏 + 顶栏 + 全宽底栏
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Window
import MusicQuick

ApplicationWindow {
    id: root
    width: 1180
    height: 760
    minimumWidth: 960
    minimumHeight: 640
    visible: true
    title: qsTr("Music Quick")
    color: Theme.bgBase
    flags: Qt.Window | Qt.FramelessWindowHint

    FileDialog {
        id: importDialog
        title: qsTr("选择音乐文件")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("音频文件 (*.mp3 *.wav *.flac *.aac)"), qsTr("所有文件 (*.*)")]
        onAccepted: app.importFiles(selectedFiles)
    }

    function goToPage(page) {
        app.currentPage = page
        if (page !== 3) {
            app.activePlaylistId = ""
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            id: topBar
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.topBarHeight
            window: root
            onSearchSubmitted: function(keyword) {
                app.searchOnline(keyword, 1)
                root.goToPage(2)
            }
        }

        Connections {
            target: app
            function onSearchKeywordChanged() {
                if (topBar.searchField.text !== app.searchKeyword) {
                    topBar.searchField.text = app.searchKeyword
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            SideNav {
                Layout.preferredWidth: Theme.sidebarWidth
                Layout.minimumWidth: Theme.sidebarWidth
                Layout.fillHeight: true
                currentPage: app.currentPage
                onNavigate: function(page) { root.goToPage(page) }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: app.currentPage

                HomePage {
                    onOpenLocalMusic: root.goToPage(1)
                }
                LocalMusicPage {
                    trackModel: app.trackModel
                    onImportClicked: importDialog.open()
                }
                SearchPage { }
                PlaylistPage { }
            }
        }

        PlaybackBar {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.playbackBarHeight
            Layout.minimumHeight: Theme.playbackBarHeight
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

    Loader {
        id: nowPlayingLoader
        anchors.fill: parent
        z: 200
        active: app.nowPlayingVisible
        source: Qt.resolvedUrl("components/NowPlayingView.qml")
        onLoaded: {
            if (item) {
                item.window = root
            }
        }
    }
}
