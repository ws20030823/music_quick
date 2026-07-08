// Main.qml — 网易云布局：左侧栏 | 右侧内容区 | 全宽底栏
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Window
import MusicQuick

ApplicationWindow {
    id: root
    width: 1050
    height: 720
    minimumWidth: 900
    minimumHeight: 620
    visible: true
    title: qsTr("WingSound")
    color: Theme.bgBase
    flags: Qt.Window | Qt.FramelessWindowHint

    FileDialog {
        id: importDialog
        title: qsTr("选择音乐文件")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("音频文件 (*.mp3 *.wav *.flac *.aac)"), qsTr("所有文件 (*.*)")]
        onAccepted: app.importFiles(selectedFiles)
    }

    function navigateTo(page) {
        app.navigateToPage(page)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            AppSidebar {
                Layout.preferredWidth: Theme.sidebarWidth
                Layout.minimumWidth: Theme.sidebarWidth
                Layout.fillHeight: true
                window: root
                currentPage: app.currentPage
                onNavigate: function(page) { root.navigateTo(page) }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                TopBar {
                    id: topBar
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.topBarHeight
                    window: root
                    onSearchSubmitted: function(keyword) {
                        app.searchOnline(keyword, 1)
                        root.navigateTo(3)
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

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: app.currentPage

                    DiscoverPage { }
                    HomePage { }
                    LocalMusicPage {
                        trackModel: app.trackModel
                        onImportClicked: importDialog.open()
                    }
                    SearchPage { }
                    PlaylistPage { }
                    FeaturedPlaylistPage { }
                    SettingsPage { }
                }
            }
        }

        PlaybackBar {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.playbackBarHeight
            Layout.minimumHeight: Theme.playbackBarHeight
            window: root
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

    NowPlayingView {
        anchors.fill: parent
        z: 200
        window: root
    }

    ClickSpark {
        anchors.fill: parent
        z: 300
        sparkColor: Theme.accent
        sparkSize: 10
        sparkRadius: 15
        sparkCount: 8
        duration: 400
    }
}
