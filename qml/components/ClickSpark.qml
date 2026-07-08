// ClickSpark.qml — 點擊火花特效（移植自 React Bits ClickSpark）
import QtQuick
import MusicQuick

Item {
    id: root

    property color sparkColor: Theme.accent
    property int sparkSize: 10
    property int sparkRadius: 15
    property int sparkCount: 8
    property int duration: 400
    property string easing: "ease-out"
    property real extraScale: 1.0
    property bool enabled: !Theme.reduceMotion

    default property alias content: contentLayer.data

    property var _sparks: []

    Item {
        id: contentLayer
        anchors.fill: parent
    }

    Canvas {
        id: sparkCanvas
        anchors.fill: parent
        z: 2
        enabled: false

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var now = Date.now()
            var remaining = []

            for (var i = 0; i < root._sparks.length; i++) {
                var spark = root._sparks[i]
                var elapsed = now - spark.startTime
                if (elapsed >= root.duration)
                    continue

                var progress = elapsed / root.duration
                var eased = root.ease(progress)
                var distance = eased * root.sparkRadius * root.extraScale
                var lineLength = root.sparkSize * (1 - eased)
                var cosA = Math.cos(spark.angle)
                var sinA = Math.sin(spark.angle)

                var x1 = spark.x + distance * cosA
                var y1 = spark.y + distance * sinA
                var x2 = spark.x + (distance + lineLength) * cosA
                var y2 = spark.y + (distance + lineLength) * sinA

                ctx.strokeStyle = root.sparkColor
                ctx.lineWidth = 2
                ctx.beginPath()
                ctx.moveTo(x1, y1)
                ctx.lineTo(x2, y2)
                ctx.stroke()

                remaining.push(spark)
            }

            root._sparks = remaining
            if (remaining.length === 0)
                animTimer.running = false
        }
    }

    MouseArea {
        anchors.fill: parent
        z: 1
        propagateComposedEvents: true
        acceptedButtons: Qt.AllButtons
        enabled: root.enabled

        onPressed: function(mouse) {
            root.spawnSparks(mouse.x, mouse.y)
            mouse.accepted = false
        }

        onClicked: function(mouse) {
            mouse.accepted = false
        }
    }

    Timer {
        id: animTimer
        interval: 16
        repeat: true
        running: false
        onTriggered: sparkCanvas.requestPaint()
    }

    function ease(t) {
        switch (easing) {
        case "linear":
            return t
        case "ease-in":
            return t * t
        case "ease-in-out":
            return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t
        default:
            return t * (2 - t)
        }
    }

    function spawnSparks(x, y) {
        if (!enabled)
            return

        var now = Date.now()
        var batch = []
        for (var i = 0; i < sparkCount; i++) {
            batch.push({
                x: x,
                y: y,
                angle: (2 * Math.PI * i) / sparkCount,
                startTime: now
            })
        }

        _sparks = _sparks.concat(batch)
        animTimer.running = true
        sparkCanvas.requestPaint()
    }
}
