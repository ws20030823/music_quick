// CreatePlaylistDialog.qml — 新建歌单对话框
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Dialog {
    id: root
    title: qsTr("新建歌单")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias playlistName: nameField.text

    onAccepted: {
        const name = nameField.text.trim()
        if (name.length > 0) {
            app.createPlaylist(name)
        }
    }

    onOpened: {
        nameField.text = ""
        nameField.forceActiveFocus()
    }

    background: Rectangle {
        radius: Theme.radiusLg
        color: Theme.bgCard
        border.color: Theme.borderStrong
    }

    contentItem: ColumnLayout {
        spacing: 12
        width: 320

        Text {
            text: qsTr("歌单名称")
            font.pixelSize: 13
            color: Theme.textSecondary
        }

        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: qsTr("我的歌单")
            font.pixelSize: 14
            color: Theme.textPrimary
            maximumLength: 40
            onAccepted: root.accept()
            background: Rectangle {
                radius: Theme.radiusMd
                color: Theme.bgBase
                border.color: nameField.activeFocus ? Theme.accent : Theme.border
            }
        }
    }
}
