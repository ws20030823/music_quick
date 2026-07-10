// SettingsContentView.qml — 设置右侧内容区
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    property int section: 0

    StackLayout {
        anchors.fill: parent
        currentIndex: root.section

        SettingsPersonalPanel {
            width: parent.width
            height: parent.height
        }
        SettingsGeneralPanel {
            width: parent.width
            height: parent.height
        }
        SettingsSystemPanel {
            width: parent.width
            height: parent.height
        }
        SettingsThemePanel {
            width: parent.width
            height: parent.height
        }
    }
}
