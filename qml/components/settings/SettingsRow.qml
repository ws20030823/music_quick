// SettingsRow.qml — 设置项行：标题 + 说明 + 右侧控件
import QtQuick
import QtQuick.Layouts
import MusicQuick

RowLayout {
    id: root

    property string label: ""
    property string description: ""

    default property alias control: controlHost.data

    Layout.fillWidth: true
    spacing: 20

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 4

        Text {
            text: root.label
            font.pixelSize: 14
            font.bold: true
            color: Theme.textPrimary
            Layout.fillWidth: true
        }

        Text {
            visible: root.description.length > 0
            text: root.description
            font.pixelSize: 12
            color: Theme.textTertiary
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            lineHeight: 1.4
        }
    }

    Item {
        id: controlHost
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: childrenRect.width
        Layout.preferredHeight: childrenRect.height
    }
}
