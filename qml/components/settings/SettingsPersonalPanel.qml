// SettingsPersonalPanel.qml — 个人
import QtQuick
import QtQuick.Controls
import MusicQuick

SettingsPanelLayout {
    title: qsTr("个人")
    subtitle: qsTr("账户与偏好")

    SettingsCard {
        title: qsTr("个人资料")
        subtitle: qsTr("即将推出")

        Label {
            text: qsTr("此区域预留用于个人资料展示。")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            width: parent.width
        }
    }
}
