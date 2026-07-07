// PlaybackBar.qml — 网易云式底栏：左信息 | 中控制+进度 | 右音量/队列

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Rectangle {

    id: root

    color: Theme.bgCard

    implicitHeight: Theme.playbackBarHeight

    required property Window window

    property bool seeking: false

    function pointOnInteractiveControl(x, y) {
        var item = contentHost.childAt(x, y)
        while (item && item !== root) {
            if (item.suppressNowPlayingOpen === true)
                return true
            item = item.parent
        }
        return false
    }

    function openNowPlayingIfBlank(x, y) {
        if (!pointOnInteractiveControl(x, y))
            app.openNowPlaying()
    }

    // 供 QML 单元测试调用
    function isInteractiveControlAt(x, y) {
        return pointOnInteractiveControl(x, y)
    }



    Rectangle {

        anchors.top: parent.top

        width: parent.width

        height: 1

        color: Theme.border

    }

    function formatMs(ms) {

        if (ms <= 0) return "00:00"

        var total = Math.floor(ms / 1000)

        var m = Math.floor(total / 60)

        var s = total % 60

        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s

    }



    component IconTransportButton: ToolButton {

        id: ctrl

        property string iconName: ""

        property int iconSize: 20

        property bool interactive: true

        property bool suppressNowPlayingOpen: true

        property color iconColor: interactive ? Theme.textPrimary : Theme.iconMuted

        implicitWidth: 36
        implicitHeight: 36
        enabled: true

        background: Rectangle {
            radius: 18
            color: ctrl.down ? Theme.accentSoft
                 : (ctrl.hovered && ctrl.interactive ? Theme.bgHover : "transparent")
        }
        contentItem: Item {
            implicitWidth: ctrl.iconSize
            implicitHeight: ctrl.iconSize
            AppIcon {
                anchors.centerIn: parent
                width: ctrl.iconSize
                height: ctrl.iconSize
                name: ctrl.iconName
                color: ctrl.iconColor
            }
        }

    }



    component PlaybackSlider: Slider {
        id: control
        property bool interactive: true
        property bool suppressNowPlayingOpen: true

        readonly property real _fillRatio: {
            if (control.from > control.to)
                return control.value / control.from
            return control.visualPosition
        }

        enabled: true
        opacity: interactive ? 1.0 : 0.55

        handle: Rectangle {
            x: control.orientation === Qt.Vertical
                ? control.leftPadding + control.availableWidth / 2 - width / 2
                : control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: control.orientation === Qt.Vertical
                ? control.topPadding + (1.0 - control.visualPosition) * (control.availableHeight - height)
                : control.topPadding + control.availableHeight / 2 - height / 2
            implicitWidth: 12
            implicitHeight: 12
            width: implicitWidth
            height: implicitHeight
            radius: width / 2
            color: Theme.textPrimary
            border.color: Theme.borderStrong
            border.width: 1
        }

        background: Item {
            x: control.leftPadding
            y: control.topPadding
            width: control.availableWidth
            height: control.availableHeight

            Rectangle {
                id: track
                anchors.centerIn: parent
                width: control.orientation === Qt.Vertical ? 4 : parent.width
                height: control.orientation === Qt.Vertical ? parent.height : 4
                radius: 2
                color: Theme.sliderTrack

                Rectangle {
                    color: Theme.accent
                    radius: 2
                    width: control.orientation === Qt.Vertical ? parent.width : control.visualPosition * parent.width
                    height: control.orientation === Qt.Vertical
                        ? control._fillRatio * parent.height
                        : parent.height
                    anchors.left: control.orientation === Qt.Vertical ? undefined : parent.left
                    anchors.bottom: control.orientation === Qt.Vertical ? parent.bottom : undefined
                }
            }
        }
    }



    MouseArea {
        id: blankOpenLayer
        objectName: "blankOpenLayer"
        anchors.fill: parent
        z: 0
        onClicked: app.openNowPlaying()
    }

    Item {
        id: contentHost
        anchors.fill: parent
        z: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 16



        Item {
            id: trackInfoHost
            Layout.preferredWidth: 280
            Layout.maximumWidth: 320
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter

            RowLayout {
                id: trackInfoSection
                anchors.fill: parent
                spacing: 12

            Rectangle {

                width: Theme.coverSize

                height: Theme.coverSize

                radius: Theme.radiusMd

                color: Theme.bgSidebar

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

                    size: 22

                    color: Theme.iconMuted

                }

            }



            ColumnLayout {

                Layout.fillWidth: true

                spacing: 2



                Text {

                    text: app.currentTitle || qsTr("未选择歌曲")

                    font.pixelSize: 14

                    font.bold: true

                    color: Theme.textPrimary

                    elide: Text.ElideRight

                    Layout.fillWidth: true

                }

                Text {

                    text: app.currentArtist || app.currentSubtitle || qsTr("请导入并选择歌曲")

                    font.pixelSize: 12

                    color: Theme.textSecondary

                    elide: Text.ElideRight

                    Layout.fillWidth: true

                }

            }

            }

        }



        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter

            ColumnLayout {
                anchors.fill: parent
                spacing: 6

            RowLayout {

                Layout.alignment: Qt.AlignHCenter

                spacing: 12



                IconTransportButton {

                    iconName: Icons.modeIconName(app.playbackMode)

                    iconColor: !app.canControl ? Theme.iconMuted
                        : (app.playbackMode === 0 ? Theme.textPrimary : Theme.accent)

                    interactive: app.canControl

                    onClicked: { if (app.canControl) app.cyclePlaybackMode() }

                }

                IconTransportButton {

                    iconName: Icons.prev

                    interactive: app.canControl

                    onClicked: { if (app.canControl) app.playPrevious() }

                }

                IconTransportButton {

                    iconName: app.isPlaying ? Icons.pause : Icons.play

                    iconSize: 22

                    implicitWidth: 44

                    implicitHeight: 44

                    interactive: app.canControl

                    iconColor: app.canControl ? Theme.textPrimary : Theme.iconMuted

                    background: Rectangle {

                        radius: 22

                        color: parent.down ? Theme.accentSoft

                             : (parent.hovered && app.canControl ? Theme.bgHover : Theme.bgCard)

                        border.color: Theme.borderStrong

                        border.width: 1

                    }

                    onClicked: { if (app.canControl) app.togglePlayback() }

                }

                IconTransportButton {

                    iconName: Icons.next

                    interactive: app.canControl

                    onClicked: { if (app.canControl) app.playNext() }

                }

            }



            RowLayout {

                Layout.fillWidth: true

                Layout.maximumWidth: 560

                Layout.alignment: Qt.AlignHCenter

                spacing: 8



                Text {

                    text: formatMs(app.position)

                    font.pixelSize: 11

                    color: Theme.textTertiary

                    Layout.preferredWidth: 40

                }



                PlaybackSlider {

                    id: progressSlider

                    Layout.fillWidth: true

                    interactive: app.canControl && app.duration > 0

                    from: 0

                    to: app.duration > 0 ? app.duration : 1

                    value: app.position

                    live: true

                    onPressedChanged: {

                        if (!app.canControl || app.duration <= 0)

                            return

                        root.seeking = pressed

                        if (!pressed)

                            app.seek(value)

                    }

                    onValueChanged: {

                        if (pressed && app.canControl)

                            app.seek(value)

                    }

                }



                Text {

                    text: formatMs(app.duration)

                    font.pixelSize: 11

                    color: Theme.textTertiary

                    Layout.preferredWidth: 40

                    horizontalAlignment: Text.AlignRight

                }

            }

            }

        }



        RowLayout {

            Layout.preferredWidth: 88

            Layout.maximumWidth: 100

            Layout.fillHeight: true

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            spacing: 4



            Item {

                id: volumeAnchor

                property bool suppressNowPlayingOpen: true

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

                IconTransportButton {

                    id: volumeButton

                    anchors.fill: parent

                    iconName: Icons.volume

                    iconSize: 18

                    interactive: true

                    ToolTip.text: qsTr("音量")

                    onClicked: volumePopup.open()

                }



                Popup {

                    id: volumePopup

                    parent: volumeAnchor

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

                        color: Theme.bgCard

                        border.color: Theme.borderStrong

                    }



                    ColumnLayout {

                        anchors.fill: parent

                        spacing: 8



                        Text {
                            text: Math.round(app.volume)
                            font.pixelSize: 11
                            color: Theme.textSecondary
                            Layout.alignment: Qt.AlignHCenter
                        }

                        PlaybackSlider {
                            id: volumeSlider
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            orientation: Qt.Vertical
                            interactive: true
                            from: 0
                            to: 100
                            value: app.volume
                            live: true
                            WheelHandler {
                                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                                onWheel: function(event) {
                                    var step = event.angleDelta.y > 0 ? 3 : -3
                                    volumeSlider.value = Math.max(volumeSlider.from,
                                        Math.min(volumeSlider.to, volumeSlider.value + step))
                                    app.volume = Math.round(volumeSlider.value)
                                    event.accepted = true
                                }
                            }
                            onValueChanged: {
                                if (pressed)
                                    app.volume = Math.round(value)
                            }
                            onPressedChanged: {
                                if (!pressed)
                                    app.volume = Math.round(value)
                            }
                        }

                    }

                }

            }



            IconTransportButton {

                iconName: Icons.playlist

                interactive: app.trackCount > 0

                ToolTip.text: qsTr("播放队列")

                onClicked: { if (app.trackCount > 0) app.queueVisible = true }

            }

        }

        }

    }

    Connections {

        target: app

        function onPositionChanged() {

            if (!root.seeking && !progressSlider.pressed)

                progressSlider.value = app.position

        }

        function onDurationChanged() {

            progressSlider.to = app.duration > 0 ? app.duration : 1

        }

        function onVolumeChanged() {
            if (!volumeSlider.pressed)
                volumeSlider.value = app.volume
        }

    }

}

