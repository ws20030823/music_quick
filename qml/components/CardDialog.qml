// CardDialog.qml — 交互确认/表单对话框（Uiverse 卡片风格）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Popup {
    id: root

    modal: true
    padding: 0
    anchors.centerIn: Overlay.overlay
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string heading: ""
    property string description: ""
    property string primaryText: ""
    property string secondaryText: qsTr("取消")
    property bool destructive: false
    property bool showCloseButton: true
    property bool closeOnPrimary: true
    property bool closeOnSecondary: true

    default property alias body: bodySlot.data

    signal accepted()
    signal rejected()

    function accept() {
        accepted()
        if (closeOnPrimary)
            close()
    }

    function reject() {
        rejected()
        if (closeOnSecondary)
            close()
    }

    background: Item { }

    contentItem: Item {
        implicitWidth: card.implicitWidth
        implicitHeight: card.implicitHeight

        CardDialogSurface {
            id: card
            minWidth: Theme.dialogMinWidth

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.dialogContentGap

                Text {
                    Layout.fillWidth: true
                    Layout.rightMargin: root.showCloseButton ? 28 : 0
                    text: root.heading
                    visible: root.heading.length > 0
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    color: Theme.dialogHeading
                    wrapMode: Text.WordWrap
                }

                Text {
                    Layout.fillWidth: true
                    text: root.description
                    visible: root.description.length > 0
                    font.pixelSize: 14
                    font.weight: Font.Light
                    color: Theme.dialogDescription
                    wrapMode: Text.WordWrap
                    lineHeight: 1.35
                }

                ColumnLayout {
                    id: bodySlot
                    Layout.fillWidth: true
                    spacing: Theme.dialogContentGap
                }
            }

            RowLayout {
                visible: root.primaryText.length > 0 || root.secondaryText.length > 0
                Layout.fillWidth: true
                spacing: Theme.dialogButtonGap

                CardDialogButton {
                    visible: root.secondaryText.length > 0
                    Layout.fillWidth: true
                    text: root.secondaryText
                    onClicked: root.reject()
                }

                CardDialogButton {
                    visible: root.primaryText.length > 0
                    Layout.fillWidth: true
                    text: root.primaryText
                    primary: true
                    destructive: root.destructive
                    onClicked: root.accept()
                }
            }
        }

        ToolButton {
            visible: root.showCloseButton
            anchors.top: card.top
            anchors.right: card.right
            anchors.topMargin: 20
            anchors.rightMargin: 20
            implicitWidth: 28
            implicitHeight: 28
            z: 2
            onClicked: root.reject()
            background: Item {}
            contentItem: Text {
                text: "\u00D7"
                font.pixelSize: 20
                font.weight: Font.Normal
                color: parent.hovered ? Theme.dialogExitHover : Theme.dialogExit
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
