// NowPlayingView.qml — 网易云风格全屏播放页（黑胶 + 歌词 + 底栏控制）
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
    visible: opacity > 0
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

    component DarkIconButton: ToolButton {
        id: btn
        property string iconName: ""
        property int iconSize: 18
        property color normalColor: Theme.npTextMuted
        property color activeColor: Theme.npText
        implicitWidth: 36
        implicitHeight: 36
        background: Rectangle {
            radius: 18
            color: btn.down ? "#33FFFFFF" : (btn.hovered ? "#1AFFFFFF" : "transparent")
        }
        contentItem: AppIcon {
            name: btn.iconName
            size: btn.iconSize
            color: btn.hovered || btn.down ? btn.activeColor : btn.normalColor
            anchors.centerIn: parent
        }
    }

    component DarkSlider: Slider {
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
            color: Theme.npText
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
                color: Theme.npBorder
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
                    color: collapseBtn.down ? "#33FFFFFF"
                         : (collapseBtn.hovered ? "#1AFFFFFF" : "transparent")
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

            DarkIconButton {
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

            // 左侧黑胶
            Item {
                Layout.fillHeight: true
                Layout.preferredWidth: Theme.npVinylSize + 80
                Layout.maximumWidth: Theme.npVinylSize + 80
                Layout.alignment: Qt.AlignVCenter

                Item {
                    id: vinylStage
                    anchors.centerIn: parent
                    width: Theme.npVinylSize
                    height: Theme.npVinylSize

                    // 唱臂
                    Rectangle {
                        id: toneArm
                        width: 140
                        height: 8
                        radius: 4
                        color: "#E5E7EB"
                        x: vinylStage.width * 0.58
                        y: -8
                        transformOrigin: Item.TopLeft
                        rotation: app.isPlaying ? 18 : 0
                        Behavior on rotation { NumberAnimation { duration: 400; easing.type: Easing.OutCubic } }

                        Rectangle {
                            width: 18
                            height: 18
                            radius: 9
                            color: "#D1D5DB"
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // 唱片外圈
                    Item {
                        id: disc
                        anchors.centerIn: parent
                        width: Theme.npVinylSize
                        height: Theme.npVinylSize

                        RotationAnimation on rotation {
                            running: app.isPlaying
                            from: 0
                            to: 360
                            duration: 18000
                            loops: Animation.Infinite
                        }

                        Rectangle {
                            anchors.fill: parent
                            radius: width / 2
                            color: Theme.npVinyl
                            border.color: "#1F1F24"
                            border.width: 2

                            // 纹路
                            Repeater {
                                model: 5
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * (0.92 - index * 0.08)
                                    height: width
                                    radius: width / 2
                                    color: "transparent"
                                    border.color: "#151518"
                                    border.width: 1
                                }
                            }

                            // 中心标签 + 封面
                            Rectangle {
                                anchors.centerIn: parent
                                width: parent.width * 0.38
                                height: width
                                radius: width / 2
                                color: Theme.accent
                                clip: true

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
                                    size: 36
                                    color: "#FFFFFF"
                                }
                            }
                        }
                    }
                }
            }

            // 右侧信息 + 歌词
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
                        color: "#33FFFFFF"
                        border.color: Theme.npBorder
                        implicitWidth: localBadge.implicitWidth + 12
                        implicitHeight: 22

                        Text {
                            id: localBadge
                            anchors.centerIn: parent
                            text: qsTr("本地")
                            font.pixelSize: 11
                            color: Theme.npTextMuted
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 32

                    ColumnLayout {
                        spacing: 4
                        Text {
                            text: qsTr("专辑：%1").arg(app.currentAlbum || qsTr("未知专辑"))
                            font.pixelSize: 13
                            color: Theme.npTextMuted
                        }
                    }
                    ColumnLayout {
                        spacing: 4
                        Text {
                            text: qsTr("歌手：%1").arg(app.currentArtist || qsTr("未知艺术家"))
                            font.pixelSize: 13
                            color: Theme.npTextMuted
                        }
                    }
                    ColumnLayout {
                        spacing: 4
                        Text {
                            text: qsTr("来源：%1").arg(app.currentSource || qsTr("—"))
                            font.pixelSize: 13
                            color: Theme.npTextMuted
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.radiusLg
                    color: Theme.npPanel
                    border.color: Theme.npBorder
                    border.width: 1
                    clip: true

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 24
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                        Column {
                            width: parent.width
                            spacing: 16
                            topPadding: 8
                            bottomPadding: 24

                            Text {
                                width: parent.width
                                text: qsTr("暂无歌词")
                                font.pixelSize: 15
                                lineHeight: 1.8
                                color: Theme.npTextDim
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
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
                        color: Theme.npBg
                        clip: true
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

                    DarkIconButton {
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

                        DarkIconButton {
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
                        DarkIconButton {
                            iconName: Icons.prev
                            enabled: app.canControl
                            onClicked: { if (app.canControl) app.playPrevious() }
                        }
                        DarkIconButton {
                            iconName: app.isPlaying ? Icons.pause : Icons.play
                            iconSize: 22
                            implicitWidth: 44
                            implicitHeight: 44
                            enabled: app.canControl
                            background: Rectangle {
                                radius: 22
                                color: parent.down ? "#33FFFFFF" : (parent.hovered ? "#1AFFFFFF" : Theme.npBg)
                                border.color: Theme.npBorder
                                border.width: 1
                            }
                            onClicked: { if (app.canControl) app.togglePlayback() }
                        }
                        DarkIconButton {
                            iconName: Icons.next
                            enabled: app.canControl
                            onClicked: { if (app.canControl) app.playNext() }
                        }
                        DarkIconButton {
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

                        DarkSlider {
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

                        DarkIconButton {
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
