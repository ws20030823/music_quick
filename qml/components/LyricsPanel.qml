// LyricsPanel.qml — 网易云风格歌词（卡片蒙层 / 高亮 / 渐变 / 平滑滚动）
import QtQuick
import MusicQuick

Item {
    id: root

    property string lyricsText: ""
    property int positionMs: 0
    property bool canSeek: false
    property real cardShellAlpha: 0.20

    signal lineClicked(int timeMs)

    readonly property var parsedLines: parseLrc(lyricsText)
    readonly property bool hasLyrics: parsedLines.length > 0
    readonly property int activeIndex: hasLyrics ? indexForPosition(positionMs) : -1
    readonly property color shellColor: Theme.cardShellTint(cardShellAlpha)
    readonly property color shellBorder: Theme.cardShellBorder(cardShellAlpha)

    function parseLrc(raw) {
        if (!raw || raw.trim().length === 0)
            return []

        var out = []
        var lines = raw.split(/\r?\n/)
        for (var i = 0; i < lines.length; i++) {
            var line = lines[i].trim()
            if (line.length === 0)
                continue

            var text = line.replace(/\[\d+:\d+(?:\.\d+)?\]/g, "").trim()
            if (text.length === 0)
                continue

            var timeRe = /\[\d+:\d+(?:\.\d+)?\]/g
            var match
            var times = []
            while ((match = timeRe.exec(line)) !== null) {
                var inner = match[0].slice(1, -1)
                var colon = inner.indexOf(":")
                var min = parseInt(inner.slice(0, colon), 10)
                var sec = parseFloat(inner.slice(colon + 1))
                times.push(Math.round((min * 60 + sec) * 1000))
            }

            if (times.length === 0) {
                var fallbackMs = out.length > 0 ? out[out.length - 1].timeMs : 0
                out.push({ timeMs: fallbackMs, text: text })
            } else {
                for (var j = 0; j < times.length; j++)
                    out.push({ timeMs: times[j], text: text })
            }
        }

        out.sort(function(a, b) { return a.timeMs - b.timeMs })
        return out
    }

    function indexForPosition(ms) {
        if (parsedLines.length === 0)
            return 0
        var idx = 0
        for (var i = 0; i < parsedLines.length; i++) {
            if (parsedLines[i].timeMs <= ms)
                idx = i
            else
                break
        }
        return idx
    }

    function resetScroll() {
        if (!hasLyrics)
            return
        lyricsList.currentIndex = activeIndex
        lyricsList.positionViewAtIndex(activeIndex, ListView.Center)
    }

    Rectangle {
        id: card
        anchors.fill: parent
        radius: Theme.radiusLg
        color: root.shellColor
        border.color: root.shellBorder
        border.width: 1
        clip: true

        Text {
            anchors.centerIn: parent
            visible: !root.hasLyrics
            text: qsTr("暂无歌词")
            font.pixelSize: 15
            color: Theme.textTertiary
        }

        ListView {
            id: lyricsList
            anchors.fill: parent
            visible: root.hasLyrics
            model: root.parsedLines
            currentIndex: root.activeIndex
            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: height * 0.38
            preferredHighlightEnd: height * 0.62
            highlightMoveDuration: Theme.npLyricScrollMs
            highlightMoveVelocity: 1.0
            boundsBehavior: Flickable.StopAtBounds
            interactive: true
            cacheBuffer: Theme.npLyricLineHeight * 8
            spacing: 0

            header: Item {
                width: lyricsList.width
                height: lyricsList.height * 0.4
            }
            footer: Item {
                width: lyricsList.width
                height: lyricsList.height * 0.4
            }

            delegate: Item {
                id: lineItem
                width: lyricsList.width
                height: Theme.npLyricLineHeight

                readonly property bool isActive: lyricsList.currentIndex === index

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.npLyricPaddingH
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.npLyricPaddingH
                    text: modelData.text
                    wrapMode: Text.Wrap
                    font.family: Theme.npLyricFontFamily
                    font.pixelSize: lineItem.isActive ? Theme.npLyricActiveSize : Theme.npLyricInactiveSize
                    font.weight: lineItem.isActive ? Font.Bold : Font.Normal
                    color: lineItem.isActive ? Theme.npLyricActive : Theme.npLyricInactive

                    Behavior on font.pixelSize {
                        NumberAnimation { duration: Theme.npLyricAnimMs; easing.type: Easing.OutCubic }
                    }
                    Behavior on color {
                        ColorAnimation { duration: Theme.npLyricAnimMs }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: root.canSeek ? Qt.PointingHandCursor : Qt.ArrowCursor
                    enabled: root.canSeek
                    onClicked: root.lineClicked(modelData.timeMs)
                }
            }
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: Theme.npLyricFadeHeight
            z: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: root.shellColor }
                GradientStop { position: 1.0; color: "#00FFFFFF" }
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: Theme.npLyricFadeHeight
            z: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00FFFFFF" }
                GradientStop { position: 1.0; color: root.shellColor }
            }
        }
    }

    onLyricsTextChanged: Qt.callLater(resetScroll)
    onActiveIndexChanged: {
        if (activeIndex >= 0 && lyricsList.count > 0)
            lyricsList.currentIndex = activeIndex
    }
}
