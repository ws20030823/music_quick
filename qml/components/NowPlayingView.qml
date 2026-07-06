// NowPlayingView.qml — 网易云风格全屏播放页（黑胶 + 白色歌词 + 底栏控制）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Rectangle {
    id: root
    required property Window window

    anchors.fill: parent
    color: Theme.npBg

    property bool seeking: false

    function formatMs(ms) {
        if (ms <= 0) return "00:00"
        var total = Math.floor(ms / 1000)
        var m = Math.floor(total / 60)
        var s = total % 60
        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }

    opacity: app.nowPlayingVisible ? 1 : 0
    visible: app.nowPlayingVisible
    enabled: app.nowPlayingVisible

    Behavior on opacity { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

    Shortcut {
        sequence: StandardKey.Cancel
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

    component NpIconButton: ToolButton {
        id: btn
        property string iconName: ""
        property int iconSize: 18
        property color normalColor: Theme.npTextMuted
        property color activeColor: Theme.npText
        implicitWidth: 36
        implicitHeight: 36
        background: Rectangle {
            radius: 18
            color: btn.down ? Theme.accentSoft
                 : (btn.hovered ? Theme.bgHover : "transparent")
        }
        contentItem: AppIcon {
            name: btn.iconName
            size: btn.iconSize
            color: btn.hovered || btn.down ? btn.activeColor : btn.normalColor
            anchors.centerIn: parent
        }
    }

    component NpSlider: Slider {
        id: control
        property bool interactive: true
        enabled: interactive
        opacity: interactive ? 1 : 0.5

        handle: Rectangle {
            x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: control.topPadding + control.availableHeight / 2 - height / 2
            implicitWidth: 10
            implicitHeight: 10
            radius: 5
            color: Theme.sliderHandle
            border.color: Theme.borderStrong
            border.width: 1
        }
        background: Item {
            x: control.leftPadding
            y: control.topPadding
            width: control.availableWidth
            height: control.availableHeight
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                height: 3
                radius: 1.5
                color: Theme.sliderTrack
                Rectangle {
                    width: control.visualPosition * parent.width
                    height: parent.height
                    radius: 1.5
                    color: Theme.accent
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ── 顶栏 ──
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.topBarHeight
            Layout.leftMargin: 8
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
                        color: collapseBtn.hovered ? Theme.npText : Theme.npTextMuted
                        rotation: -90
                        transformOrigin: Item.Center
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: qsTr("播放器模式")
                font.pixelSize: 12
                color: Theme.npTextMuted
            }

            NpIconButton {
                iconName: Icons.modeIconName(app.playbackMode)
                iconSize: 16
                enabled: app.canControl
                onClicked: { if (app.canControl) app.cyclePlaybackMode() }
                contentItem: AppIcon {
                    name: Icons.modeIconName(app.playbackMode)
                    size: 16
                    color: app.playbackMode === 0 ? Theme.npTextMuted : Theme.accent
                    anchors.centerIn: parent
                }
            }

            Item { Layout.preferredWidth: 12 }

            WindowControls {
                window: root.window
            }
        }

        // ── 主内容：黑胶 + 信息/歌词 ──
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 48
            Layout.rightMargin: 48
            Layout.bottomMargin: 16
            spacing: 56

            // 左侧黑胶舞台
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
                    color: Theme.npVinylStage
                }

                VinylDisc {
                    anchors.centerIn: parent
                    playing: app.isPlaying
                    coverUrl: app.currentCoverUrl
                    hasCover: app.hasCover
                    discSize: Theme.npVinylSize
                }
            }

            // 右侧信息 + 歌词（白色主题）
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
                        color: Theme.npText
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
                        color: Theme.npTextMuted
                    }
                    Text {
                        text: qsTr("歌手：%1").arg(app.currentArtist || qsTr("未知艺术家"))
                        font.pixelSize: 13
                        color: Theme.npTextMuted
                    }
                    Text {
                        text: qsTr("来源：%1").arg(app.currentSource || qsTr("—"))
                        font.pixelSize: 13
                        color: Theme.npTextMuted
                    }
                    Item { Layout.fillWidth: true }
                }

                // 歌词面板
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        anchors.fill: parent
                        anchors.topMargin: 3
                        radius: Theme.radiusLg
                        color: Theme.npLyricsShadow
                    }

                    Rectangle {
                        id: lyricsCard
                        anchors.fill: parent
                        radius: Theme.radiusLg
                        color: Theme.npLyricsBg
                        border.color: Theme.npBorder
                        border.width: 1
                        clip: true

                        ScrollView {
                            anchors.fill: parent
                            clip: true
                            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                            Column {
                                width: lyricsCard.width
                                spacing: 14
                                topPadding: 28
                                leftPadding: 28
                                rightPadding: 28
                                bottomPadding: 28

                                Text {
                                    width: parent.width - parent.leftPadding - parent.rightPadding
                                    text: app.currentLyrics.length > 0 ? app.currentLyrics : qsTr("暂无歌词")
                                    font.pixelSize: 16
                                    lineHeight: 1.85
                                    color: app.currentLyrics.length > 0 ? Theme.npText : Theme.npTextDim
                                    horizontalAlignment: app.currentLyrics.length > 0 ? Text.AlignLeft : Text.AlignHCenter
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }
        }

        // ── 底栏播放控制 ──
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.playbackBarHeight
            color: Theme.npPanel

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.npBorder
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 16

                RowLayout {
                    Layout.preferredWidth: 300
                    Layout.maximumWidth: 340
                    spacing: 12

                    Rectangle {
                        width: Theme.coverSize
                        height: Theme.coverSize
                        radius: Theme.radiusMd
                        color: Theme.bgBase
                        clip: true
                        border.color: Theme.npBorder
                        border.width: 1
                        Image {
                            anchors.fill: parent
                            source: app.currentCoverUrl
                            cache: false
                            fillMode: Image.PreserveAspectCrop
                            visible: app.hasCover
                        }
                        AppIcon {
                            anchors.centerIn: parent
                            visible: !app.hasCover
                            name: Icons.play
                            size: 20
                            color: Theme.npTextDim
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: app.currentTitle || qsTr("未选择歌曲")
                            font.pixelSize: 14
                            font.bold: true
                            color: Theme.npText
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Text {
                            text: app.currentArtist || qsTr("—")
                            font.pixelSize: 12
                            color: Theme.npTextMuted
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    NpIconButton {
                        iconName: app.isCurrentTrackLiked ? Icons.heartFilled : Icons.heart
                        iconSize: 16
                        enabled: app.currentSongId.length > 0
                        onClicked: app.toggleCurrentLike()
                        contentItem: AppIcon {
                            name: app.isCurrentTrackLiked ? Icons.heartFilled : Icons.heart
                            size: 16
                            color: app.isCurrentTrackLiked ? Theme.accent : Theme.npTextMuted
                            anchors.centerIn: parent
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 16

                        NpIconButton {
                            iconName: Icons.modeIconName(app.playbackMode)
                            enabled: app.canControl
                            onClicked: { if (app.canControl) app.cyclePlaybackMode() }
                            contentItem: AppIcon {
                                name: Icons.modeIconName(app.playbackMode)
                                size: 18
                                color: app.playbackMode === 0 ? Theme.npTextMuted : Theme.accent
                                anchors.centerIn: parent
                            }
                        }
                        NpIconButton {
                            iconName: Icons.prev
                            enabled: app.canControl
                            onClicked: { if (app.canControl) app.playPrevious() }
                        }
                        NpIconButton {
                            iconName: app.isPlaying ? Icons.pause : Icons.play
                            iconSize: 22
                            implicitWidth: 44
                            implicitHeight: 44
                            enabled: app.canControl
                            background: Rectangle {
                                radius: 22
                                color: parent.down ? Theme.accentSoft
                                     : (parent.hovered ? Theme.bgHover : Theme.bgBase)
                                border.color: Theme.npBorder
                                border.width: 1
                            }
                            onClicked: { if (app.canControl) app.togglePlayback() }
                        }
                        NpIconButton {
                            iconName: Icons.next
                            enabled: app.canControl
                            onClicked: { if (app.canControl) app.playNext() }
                        }
                        NpIconButton {
                            iconName: Icons.playlist
                            enabled: app.canControl
                            onClicked: { if (app.canControl) app.queueVisible = true }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.maximumWidth: 520
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 8

                        Text {
                            text: formatMs(app.position)
                            font.pixelSize: 11
                            color: Theme.npTextDim
                            Layout.preferredWidth: 40
                        }

                        NpSlider {
                            id: npProgress
                            Layout.fillWidth: true
                            interactive: app.canControl && app.duration > 0
                            from: 0
                            to: app.duration > 0 ? app.duration : 1
                            value: app.position
                            live: true
                            onPressedChanged: {
                                if (!app.canControl || app.duration <= 0) return
                                root.seeking = pressed
                                if (!pressed) app.seek(value)
                            }
                            onValueChanged: {
                                if (pressed && app.canControl) app.seek(value)
                            }
                        }

                        Text {
                            text: formatMs(app.duration)
                            font.pixelSize: 11
                            color: Theme.npTextDim
                            Layout.preferredWidth: 40
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }

                RowLayout {
                    Layout.preferredWidth: 72
                    spacing: 4

                    Item {
                        id: npVolumeAnchor
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 36

                        WheelHandler {
                            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                            onWheel: function(event) {
                                var step = event.angleDelta.y > 0 ? 3 : -3
                                app.volume = Math.max(0, Math.min(100, app.volume + step))
                                event.accepted = true
                            }
                        }

                        NpIconButton {
                            anchors.fill: parent
                            iconName: Icons.volume
                            onClicked: npVolumePopup.open()
                        }

                        Popup {
                            id: npVolumePopup
                            parent: npVolumeAnchor
                            x: (parent.width - width) / 2
                            y: -height - 10
                            width: 44
                            height: 132
                            padding: 10
                            modal: false
                            focus: true
                            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
                            background: Rectangle {
                                radius: Theme.radiusMd
                                color: Theme.npPanel
                                border.color: Theme.npBorder
                            }
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 8
                                Text {
                                    text: Math.round(app.volume)
                                    font.pixelSize: 11
                                    color: Theme.npTextMuted
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Slider {
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    orientation: Qt.Vertical
                                    from: 0
                                    to: 100
                                    value: app.volume
                                    WheelHandler {
                                        onWheel: function(event) {
                                            var step = event.angleDelta.y > 0 ? 3 : -3
                                            value = Math.max(from, Math.min(to, value + step))
                                            app.volume = Math.round(value)
                                            event.accepted = true
                                        }
                                    }
                                    onMoved: app.volume = Math.round(value)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: app
        function onPositionChanged() {
            if (!root.seeking && !npProgress.pressed)
                npProgress.value = app.position
        }
        function onDurationChanged() {
            npProgress.to = app.duration > 0 ? app.duration : 1
        }
    }
}
