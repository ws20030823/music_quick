// NowPlayingView.qml — 全屏播放页（与主界面共用壁纸 + 卡片蒙层）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Item {
    id: root
    required property Window window

    anchors.fill: parent
    opacity: app.nowPlayingVisible ? 1 : 0
    visible: app.nowPlayingVisible
    enabled: app.nowPlayingVisible

    Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

    // 拦截全部指针事件，避免穿透到主页面控件
    MouseArea {
        anchors.fill: parent
        z: -1
        acceptedButtons: Qt.AllButtons
        onWheel: function(event) { event.accepted = true }
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        enabled: app.nowPlayingVisible
        onActivated: app.nowPlayingVisible = false
    }

    Connections {
        target: app
        function onNowPlayingVisibleChanged() {
            if (app.nowPlayingVisible)
                root.forceActiveFocus()
        }
    }

    WallpaperBackground {
        anchors.fill: parent
        wallpaperSource: app.hasHomeWallpaper ? app.homeWallpaperUrl : ""
        wallpaperOpacity: app.uiBackgroundOpacity
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.shellPadding
        spacing: Theme.cardGap

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.topBarHeight
            Layout.leftMargin: 4
            Layout.rightMargin: 0
            spacing: 8

            ToolButton {
                id: collapseBtn
                implicitWidth: 36
                implicitHeight: 36
                ToolTip.text: qsTr("收起")
                onClicked: app.nowPlayingVisible = false
                background: Rectangle {
                    radius: 18
                    color: collapseBtn.down ? Theme.accentSoft
                         : (collapseBtn.hovered ? Theme.bgHover : "transparent")
                }
                contentItem: Item {
                    width: 16
                    height: 16
                    AppIcon {
                        anchors.centerIn: parent
                        name: Icons.chevronLeft
                        size: 16
                        color: Theme.textPrimary
                        rotation: -90
                        transformOrigin: Item.Center
                    }
                }
            }

            Item { Layout.fillWidth: true }

            WindowControls {
                window: root.window
            }
        }

        SurfaceCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            cardShellAlpha: app.uiCardShellOpacity

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                anchors.topMargin: 8
                anchors.bottomMargin: 8
                spacing: 48

                Item {
                    Layout.fillHeight: true
                    Layout.preferredWidth: Theme.npVinylSize + 96
                    Layout.maximumWidth: Theme.npVinylSize + 96
                    Layout.alignment: Qt.AlignVCenter

                    Rectangle {
                        anchors.centerIn: parent
                        width: Theme.npVinylSize + 48
                        height: width
                        radius: width / 2
                        color: Theme.cardShellTint(app.uiCardShellOpacity * 0.55)
                        border.color: Theme.cardShellBorder(app.uiCardShellOpacity * 0.55)
                        border.width: 1
                    }

                    VinylDisc {
                        anchors.centerIn: parent
                        playing: app.isPlaying
                        coverUrl: app.currentCoverUrl
                        hasCover: app.hasCover
                        discSize: Theme.npVinylSize
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 20

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: app.currentTitle || qsTr("未选择歌曲")
                            font.pixelSize: 32
                            font.bold: true
                            color: Theme.textPrimary
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            maximumLineCount: 2
                            wrapMode: Text.Wrap
                        }

                        Rectangle {
                            visible: !app.isOnlinePlayback && app.currentTitle.length > 0
                            Layout.alignment: Qt.AlignTop
                            radius: 4
                            color: Theme.accentSoft
                            border.color: Theme.borderStrong
                            implicitWidth: localBadge.implicitWidth + 12
                            implicitHeight: 22

                            Text {
                                id: localBadge
                                anchors.centerIn: parent
                                text: qsTr("本地")
                                font.pixelSize: 11
                                color: Theme.accent
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 32

                        Text {
                            text: qsTr("专辑：%1").arg(app.currentAlbum || qsTr("未知专辑"))
                            font.pixelSize: 13
                            color: Theme.textSecondary
                        }
                        Text {
                            text: qsTr("歌手：%1").arg(app.currentArtist || qsTr("未知艺术家"))
                            font.pixelSize: 13
                            color: Theme.textSecondary
                        }
                        Text {
                            text: qsTr("来源：%1").arg(app.currentSource || qsTr("—"))
                            font.pixelSize: 13
                            color: Theme.textSecondary
                        }
                        Item { Layout.fillWidth: true }
                    }

                    LyricsPanel {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        cardShellAlpha: app.uiCardShellOpacity
                        lyricsText: app.currentLyrics
                        positionMs: app.position
                        canSeek: app.canControl && app.duration > 0
                        onLineClicked: function(timeMs) {
                            if (app.canControl)
                                app.seek(timeMs)
                        }
                    }
                }
            }
        }

        SurfaceCard {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.playbackBarHeight
            Layout.minimumHeight: Theme.playbackBarHeight
            cardShellAlpha: app.uiCardShellOpacity

            PlaybackBar {
                anchors.fill: parent
                window: root.window
                showPlaybackMode: false
            }
        }
    }
}
