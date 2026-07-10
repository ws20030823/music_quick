// CreatePlaylistDialog.qml — 新建歌单对话框
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

CardDialog {
    id: root

    parent: Overlay.overlay
    heading: qsTr("新建歌单")
    description: qsTr("为你的音乐收藏创建一个新列表")
    primaryText: qsTr("创建")
    secondaryText: qsTr("取消")
    closeOnPrimary: false

    property alias playlistName: nameField.text

    onAccepted: {
        const name = nameField.text.trim()
        if (name.length > 0) {
            app.createPlaylist(name)
            close()
        }
    }

    onOpened: {
        nameField.text = ""
        nameField.forceActiveFocus()
    }

    TextField {
        Layout.fillWidth: true
        id: nameField
        placeholderText: qsTr("我的歌单")
        font.pixelSize: 14
        color: Theme.textPrimary
        maximumLength: 40
        onAccepted: root.accept()
        background: Rectangle {
            radius: Theme.dialogButtonRadius
            color: Theme.bgBase
            border.color: nameField.activeFocus ? Theme.accent : Theme.borderStrong
        }
    }
}
