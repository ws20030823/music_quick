// InfiniteMenu.qml — 3D 球面歌单菜单（移植自 React Bits InfiniteMenu）
import QtQuick
import MusicQuick

Item {
    id: root

    property var items: []
    property real menuScale: 1.0
    property bool immersiveDark: false
    property int slotCount: 20

    property int activeIndex: 0
    property bool isMoving: false

    property real rotYaw: 0
    property real rotPitch: 0
    property real velocityYaw: 0
    property real velocityPitch: 0

    readonly property real sphereRadius: 2.0 * menuScale
    readonly property real cameraDistance: 3.0 * menuScale
    readonly property var slotPositions: computeSpherePositions(slotCount, sphereRadius)
    readonly property var activeItem: items.length > 0 ? items[activeIndex] : null
    readonly property real minDim: Math.min(width, height)

    signal itemActivated(string id)

    property var _discStates: []

    function itemImage(index) {
        if (index < 0 || index >= items.length)
            return ""
        var item = items[index]
        return item.image !== undefined ? item.image : (item.coverUrl !== undefined ? item.coverUrl : "")
    }

    function itemTitle(index) {
        if (index < 0 || index >= items.length)
            return ""
        return items[index].title !== undefined ? items[index].title : ""
    }

    function itemDescription(index) {
        if (index < 0 || index >= items.length)
            return ""
        var item = items[index]
        return item.description !== undefined ? item.description : (item.subtitle !== undefined ? item.subtitle : "")
    }

    function itemId(index) {
        if (index < 0 || index >= items.length)
            return ""
        return items[index].id !== undefined ? items[index].id : ""
    }

    function computeSpherePositions(count, radius) {
        var positions = []
        var golden = Math.PI * (3.0 - Math.sqrt(5.0))
        for (var i = 0; i < count; ++i) {
            var y = 1.0 - (i / Math.max(1, count - 1)) * 2.0
            var r = Math.sqrt(Math.max(0.0, 1.0 - y * y))
            var theta = golden * i
            positions.push({
                x: Math.cos(theta) * r * radius,
                y: y * radius,
                z: Math.sin(theta) * r * radius
            })
        }
        return positions
    }

    function rotatePoint(point, yaw, pitch) {
        var cosY = Math.cos(yaw)
        var sinY = Math.sin(yaw)
        var x1 = point.x * cosY + point.z * sinY
        var z1 = -point.x * sinY + point.z * cosY

        var cosX = Math.cos(pitch)
        var sinX = Math.sin(pitch)
        return {
            x: x1,
            y: point.y * cosX - z1 * sinX,
            z: point.y * sinX + z1 * cosX
        }
    }

    function updateDiscStates() {
        if (items.length === 0 || width <= 0 || height <= 0) {
            _discStates = []
            return
        }

        var centerX = width * 0.5
        var centerY = height * 0.5
        var perspective = cameraDistance
        var dim = minDim
        var nextStates = []
        var bestZ = -Infinity
        var bestItemIndex = 0

        for (var slot = 0; slot < slotCount; ++slot) {
            var itemIndex = slot % items.length
            var base = slotPositions[slot]
            var world = rotatePoint(base, rotYaw, rotPitch)
            var depth = world.z + cameraDistance
            var proj = perspective / Math.max(0.28, depth)
            var spread = dim * 0.40 * proj
            var screenX = centerX + world.x * spread
            var screenY = centerY - world.y * spread
            var facing = Math.max(0.0, (world.z / sphereRadius + 1.0) * 0.5)
            var idleBoost = root.isMoving ? 0.0 : 0.05
            var baseSize = 0.07 + idleBoost + Math.pow(facing, 1.4) * 0.30
            var discSize = dim * baseSize * (root.isMoving ? 0.68 : 1.0)

            nextStates.push({
                itemIndex: itemIndex,
                screenX: screenX,
                screenY: screenY,
                discSize: discSize,
                opacity: Math.min(1.0, 0.20 + facing * 0.82),
                z: world.z,
                visible: depth > 0.15 && discSize > 12
                         && (!root.isMoving || facing > 0.22)
            })

            if (world.z > bestZ) {
                bestZ = world.z
                bestItemIndex = itemIndex
            }
        }

        _discStates = nextStates
        if (activeIndex !== bestItemIndex)
            activeIndex = bestItemIndex
    }

    function snapToActive() {
        if (items.length === 0 || Theme.reduceMotion)
            return

        var bestSlot = 0
        var bestZ = -Infinity
        for (var s = 0; s < slotCount; ++s) {
            if (s % items.length !== activeIndex)
                continue
            var world = rotatePoint(slotPositions[s], rotYaw, rotPitch)
            if (world.z > bestZ) {
                bestZ = world.z
                bestSlot = s
            }
        }

        var base = slotPositions[bestSlot]
        var targetYaw = Math.atan2(-base.x, base.z)
        var horizontal = Math.sqrt(base.x * base.x + base.z * base.z)
        var targetPitch = Math.atan2(base.y, horizontal)

        rotYaw += (targetYaw - rotYaw) * 0.14
        rotPitch += (targetPitch - rotPitch) * 0.14
        velocityYaw *= 0.82
        velocityPitch *= 0.82
    }

    Rectangle {
        anchors.fill: parent
        color: root.immersiveDark ? "#0B0B0F" : "transparent"
        z: -2
    }

    Timer {
        id: frameTimer
        interval: 16
        running: root.visible && !Theme.reduceMotion
        repeat: true
        onTriggered: {
            if (!dragArea.pressed) {
                root.rotYaw += root.velocityYaw
                root.rotPitch += root.velocityPitch
                root.velocityYaw *= 0.94
                root.velocityPitch *= 0.94

                var moving = dragArea.pressed
                    || Math.abs(root.velocityYaw) > 0.0008
                    || Math.abs(root.velocityPitch) > 0.0008
                if (root.isMoving !== moving)
                    root.isMoving = moving

                if (!moving)
                    root.snapToActive()
            }
            root.updateDiscStates()
        }
    }

    onVisibleChanged: if (visible) updateDiscStates()
    onWidthChanged: updateDiscStates()
    onHeightChanged: updateDiscStates()
    onItemsChanged: {
        activeIndex = 0
        updateDiscStates()
    }

    Component.onCompleted: updateDiscStates()

    Repeater {
        id: discRepeater
        model: root.slotCount

        delegate: Item {
            id: discHost
            required property int index
            readonly property var state: root._discStates[index] || ({
                itemIndex: 0,
                screenX: -9999,
                screenY: -9999,
                discSize: 0,
                opacity: 0,
                z: -999,
                visible: false
            })
            readonly property int itemIndex: state.itemIndex
            readonly property bool isActiveItem: itemIndex === root.activeIndex
                    && Math.abs(state.z - root.sphereRadius) < 0.55

            width: state.discSize
            height: width
            x: state.screenX - width * 0.5
            y: state.screenY - height * 0.5
            z: state.z
            opacity: state.opacity
            visible: state.visible

            Rectangle {
                anchors.fill: parent
                radius: width * 0.5
                color: root.immersiveDark ? "#1A1A1F" : Theme.bgCard
                clip: true
                border.color: discHost.isActiveItem && !root.isMoving
                            ? Theme.accent
                            : (root.immersiveDark ? "#33FFFFFF" : Theme.borderStrong)
                border.width: discHost.isActiveItem && !root.isMoving ? 3 : 1

                Image {
                    anchors.fill: parent
                    source: root.itemImage(discHost.itemIndex)
                    fillMode: Image.PreserveAspectCrop
                    smooth: true
                    asynchronous: true
                }
            }
        }
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        z: 50
        acceptedButtons: Qt.LeftButton
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

        property real lastX: 0
        property real lastY: 0

        onPressed: function(mouse) {
            lastX = mouse.x
            lastY = mouse.y
            root.isMoving = true
        }

        onReleased: {
            root.isMoving = Math.abs(root.velocityYaw) > 0.001 || Math.abs(root.velocityPitch) > 0.001
            root.updateDiscStates()
        }

        onPositionChanged: function(mouse) {
            if (!pressed)
                return

            var dx = mouse.x - lastX
            var dy = mouse.y - lastY
            lastX = mouse.x
            lastY = mouse.y

            var yawDelta = dx * 0.007
            var pitchDelta = dy * 0.007
            root.rotYaw += yawDelta
            root.rotPitch = Math.max(-1.35, Math.min(1.35, root.rotPitch + pitchDelta))
            root.velocityYaw = yawDelta * 0.7
            root.velocityPitch = pitchDelta * 0.7
            root.updateDiscStates()
        }
    }

    Text {
        id: faceTitle
        anchors.left: parent.left
        anchors.leftMargin: Math.max(Theme.pagePadding, root.minDim * 0.04)
        anchors.verticalCenter: parent.verticalCenter
        x: root.isMoving ? -28 : 0
        width: Math.min(parent.width * 0.28, 320)
        text: root.itemTitle(root.activeIndex)
        wrapMode: Text.WordWrap
        font.pixelSize: Math.min(46, Math.max(26, root.minDim * 0.055))
        font.bold: true
        font.family: Theme.npLyricFontFamily
        color: root.immersiveDark ? "#FFFFFF" : Theme.textPrimary
        opacity: root.isMoving ? 0 : 1
        z: 60
        Behavior on opacity {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500; easing.type: Easing.OutCubic }
        }
        Behavior on x {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500; easing.type: Easing.OutCubic }
        }
    }

    Text {
        id: faceDescription
        anchors.right: parent.right
        anchors.rightMargin: Math.max(Theme.pagePadding, root.minDim * 0.04)
        anchors.verticalCenter: parent.verticalCenter
        x: root.isMoving ? 28 : 0
        width: Math.min(parent.width * 0.22, 220)
        horizontalAlignment: Text.AlignRight
        text: root.itemDescription(root.activeIndex)
        wrapMode: Text.WordWrap
        font.pixelSize: Math.min(20, Math.max(14, root.minDim * 0.022))
        color: root.immersiveDark ? "#B8B8C0" : Theme.textSecondary
        opacity: root.isMoving ? 0 : 1
        z: 60
        Behavior on opacity {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500; easing.type: Easing.OutCubic }
        }
        Behavior on x {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        id: actionButton
        width: Math.max(52, root.minDim * 0.07)
        height: width
        radius: width * 0.5
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.isMoving ? -90 : Math.max(28, root.minDim * 0.06)
        color: Theme.accent
        border.color: root.immersiveDark ? "#000000" : Theme.textPrimary
        border.width: 4
        opacity: root.isMoving ? 0 : 1
        scale: root.isMoving ? 0.55 : 1.0
        z: 70

        Behavior on anchors.bottomMargin {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500; easing.type: Easing.OutCubic }
        }
        Behavior on opacity {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500 }
        }
        Behavior on scale {
            enabled: !Theme.reduceMotion
            NumberAnimation { duration: root.isMoving ? 100 : 500; easing.type: Easing.OutBack }
        }

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 1
            text: "\u2197"
            font.pixelSize: parent.width * 0.42
            color: "#FFFFFF"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            enabled: !root.isMoving && root.items.length > 0
            onClicked: {
                var id = root.itemId(root.activeIndex)
                if (id !== "")
                    root.itemActivated(id)
            }
        }
    }
}
