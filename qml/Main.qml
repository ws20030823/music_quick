// Main.qml — 卡片式布局：侧栏卡片 | 主内容卡片 | 底栏卡片
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
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    readonly property real skinCardOpacity: app.uiBackgroundOpacity

    background: WallpaperBackground {
        wallpaperSource: app.hasHomeWallpaper ? app.homeWallpaperUrl : ""
        skinOpacity: app.uiBackgroundOpacity
    }

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
        anchors.margins: Theme.shellPadding
        spacing: Theme.cardGap

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.cardGap

            SurfaceCard {
                id: sidebarCard
                Layout.preferredWidth: Theme.sidebarWidth
                Layout.minimumWidth: Theme.sidebarWidth
                Layout.fillHeight: true
                cardOpacity: root.skinCardOpacity
                clip: true

                AppSidebar {
                    id: appSidebar
                    anchors.fill: parent
                    window: root
                    currentPage: app.currentPage
                    enabled: !app.settingsVisible
                    opacity: app.settingsVisible ? 0 : 1
                    Behavior on opacity {
                        enabled: !Theme.reduceMotion
                        NumberAnimation { duration: 220 }
                    }
                    onNavigate: function(page) { root.navigateTo(page) }
                }

                StaggeredMenu {
                    id: settingsMenu
                    anchors.fill: parent
                    z: 2
                    items: [
                        { label: qsTr("个人") },
                        { label: qsTr("常规") },
                        { label: qsTr("系统") },
                        { label: qsTr("换肤") }
                    ]
                    colors: ["#C8D9F0", Theme.accent]
                    position: "left"
                    opened: app.settingsVisible
                    currentIndex: app.settingsSection
                    accentColor: Theme.accent
                    showCloseButton: true
                    cardOpacity: root.skinCardOpacity
                    onItemSelected: function(index) { app.settingsSection = index }
                    onCloseRequested: app.closeSettings()
                }
            }

            SurfaceCard {
                Layout.fillWidth: true
                Layout.fillHeight: true
                cardOpacity: root.skinCardOpacity

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

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        StackLayout {
                            id: mainPages
                            anchors.fill: parent
                            visible: !app.settingsVisible
                            enabled: !app.settingsVisible
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
                        }

                        SettingsContentView {
                            anchors.fill: parent
                            visible: app.settingsVisible
                            section: app.settingsSection
                        }
                    }
                }
            }
        }

        SurfaceCard {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.playbackBarHeight
            Layout.minimumHeight: Theme.playbackBarHeight
            cardOpacity: root.skinCardOpacity

            PlaybackBar {
                anchors.fill: parent
                window: root
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
