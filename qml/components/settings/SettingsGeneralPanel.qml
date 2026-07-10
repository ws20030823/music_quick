// SettingsGeneralPanel.qml — 常规
import QtQuick
import QtQuick.Controls
import MusicQuick

SettingsPanelLayout {
    title: qsTr("常规")
    subtitle: qsTr("启动与默认行为")

    SettingsCard {
        title: qsTr("启动")
        subtitle: qsTr("控制应用启动时的行为")

        SettingsRow {
            label: qsTr("开机自启")
            description: qsTr("系统启动时自动打开 WingSound（后续版本启用）")

            Switch {
                checked: app.launchAtStartup
                enabled: false
                onToggled: app.launchAtStartup = checked
            }
        }
    }
}
