// CardDialogButton.qml — 卡片对话框按钮
import QtQuick
import QtQuick.Controls
import MusicQuick

Button {
    id: root

    property bool primary: false
    property bool destructive: false

    implicitHeight: Theme.dialogButtonHeight
    hoverEnabled: true

    background: Rectangle {
        radius: Theme.dialogButtonRadius
        color: {
            if (!root.primary)
                return root.down || root.hovered ? Theme.dialogSecondaryHover : Theme.dialogSecondary
            if (root.destructive)
                return root.down || root.hovered ? Theme.dialogPrimaryHover : Theme.dialogPrimary
            return root.down || root.hovered ? Theme.accent : Theme.accent
        }
    }

    contentItem: Text {
        text: root.text
        font.pixelSize: 14
        font.weight: Font.DemiBold
        color: root.primary ? "#FFFFFF" : "#1B1B1B"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
