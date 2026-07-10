// SettingsSystemPanel.qml — 系统（缓存与存储）
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import MusicQuick

SettingsPanelLayout {
    id: root

    title: qsTr("系统")
    subtitle: qsTr("缓存、存储空间与播放说明")

    FolderDialog {
        id: cacheFolderDialog
        title: qsTr("选择缓存目录")
        onAccepted: {
            const dir = selectedFolder.toLocalFile()
            if (dir.length > 0)
                app.cacheDirectory = dir
        }
    }

    CardDialog {
        id: clearCacheDialog
        parent: Overlay.overlay
        heading: qsTr("清空缓存")
        description: qsTr("将删除所有已缓存的音频与封面文件，不影响歌单列表。确定继续？")
        primaryText: qsTr("清空")
        secondaryText: qsTr("取消")
        destructive: true
        onAccepted: app.clearMediaCache()
    }

    SettingsCard {
        title: qsTr("缓存与存储")
        subtitle: qsTr("在线音乐会先下载到本地再播放；已缓存的歌曲下次可更快播放")

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("缓存目录")
                    font.pixelSize: 13
                    font.bold: true
                    color: Theme.textPrimary
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        radius: Theme.radiusMd
                        color: Theme.bgCard
                        border.color: Theme.borderStrong

                        Text {
                            anchors.fill: parent
                            anchors.margins: 12
                            verticalAlignment: Text.AlignVCenter
                            text: app.cacheDirectory
                            font.pixelSize: 12
                            color: Theme.textSecondary
                            elide: Text.ElideMiddle
                        }
                    }

                    ToolButton {
                        text: qsTr("浏览…")
                        implicitHeight: 40
                        onClicked: cacheFolderDialog.open()
                        background: Rectangle {
                            radius: Theme.radiusMd
                            color: parent.hovered ? Theme.bgHover : Theme.bgCard
                            border.color: Theme.borderStrong
                        }
                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 13
                            color: Theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("最大缓存容量")
                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: app.cacheMaxSizeMb + " MB"
                        font.pixelSize: 13
                        color: Theme.accent
                        font.bold: true
                    }
                }

                Slider {
                    id: cacheSizeSlider
                    Layout.fillWidth: true
                    from: 128
                    to: 4096
                    stepSize: 128
                    value: app.cacheMaxSizeMb
                    onPressedChanged: {
                        if (!pressed)
                            app.cacheMaxSizeMb = Math.round(value / 128) * 128
                    }
                    onMoved: app.cacheMaxSizeMb = Math.round(value / 128) * 128
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

            SettingsRow {
                label: qsTr("已用空间")
                description: app.cacheUsedText + " / " + app.cacheMaxSizeMb + " MB"

                ToolButton {
                    text: qsTr("清空缓存")
                    implicitHeight: 36
                    onClicked: clearCacheDialog.open()
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? "#0DEF4444" : Theme.bgCard
                        border.color: "#FCA5A5"
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 13
                        color: "#DC2626"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    SettingsCard {
        title: qsTr("播放与缓存说明")

        Text {
            Layout.fillWidth: true
            text: qsTr("• 立即播放：音频已缓存在本地\n• 正在缓冲：在线歌曲需先下载\n• 封面缓存：收藏后封面会保存到本地")
            font.pixelSize: 12
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            lineHeight: 1.55
        }
    }

    Connections {
        target: app
        function onCacheMaxSizeMbChanged() {
            if (!cacheSizeSlider.pressed)
                cacheSizeSlider.value = app.cacheMaxSizeMb
        }
        function onCacheSettingsChanged() {
            app.refreshCacheUsage()
        }
    }

    Component.onCompleted: app.refreshCacheUsage()
}
