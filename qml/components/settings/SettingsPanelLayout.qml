// SettingsPanelLayout.qml — 设置页滚动布局（正确宽度 + 居中）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    property string title: ""
    property string subtitle: ""
    default property alias content: bodyColumn.data

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: pageColumn
            width: Math.min(root.width, Theme.settingsContentMaxWidth)
            x: root.width > width ? (root.width - width) / 2 : Theme.pagePadding
            y: Theme.pagePadding
            spacing: Theme.sectionGap

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    text: root.title
                    font.pixelSize: 26
                    font.bold: true
                    color: Theme.textPrimary
                }

                Text {
                    visible: root.subtitle.length > 0
                    text: root.subtitle
                    font.pixelSize: 13
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    lineHeight: 1.35
                }
            }

            ColumnLayout {
                id: bodyColumn
                Layout.fillWidth: true
                spacing: Theme.sectionGap
            }

            Item { Layout.preferredHeight: Theme.pagePadding }
        }
    }
}
