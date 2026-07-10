// CardDialogSurface.qml — 卡片对话框视觉容器（圆角 + 阴影）
import QtQuick
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    property real minWidth: Theme.dialogMinWidth
    default property alias content: bodyColumn.data

    implicitWidth: Math.max(minWidth, bodyColumn.implicitWidth + Theme.dialogPadding * 2)
    implicitHeight: bodyColumn.implicitHeight + Theme.dialogPadding * 2

    Rectangle {
        id: shadowPlate
        anchors.fill: cardBody
        anchors.leftMargin: Theme.dialogShadowOffset
        anchors.topMargin: Theme.dialogShadowOffset
        radius: Theme.dialogRadius
        color: Theme.dialogShadow
        visible: !Theme.reduceMotion
    }

    Rectangle {
        id: cardBody
        anchors.fill: parent
        radius: Theme.dialogRadius
        color: Theme.dialogBg
    }

    ColumnLayout {
        id: bodyColumn
        anchors.fill: parent
        anchors.margins: Theme.dialogPadding
        spacing: Theme.dialogGap
    }
}
