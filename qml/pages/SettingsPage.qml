// SettingsPage.qml — 播放器设置（缓存 / 存储）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import MusicQuick

Item {
    id: root

    FolderDialog {
        id: cacheFolderDialog
        title: qsTr("选择缓存目录")
        onAccepted: {
            const dir = selectedFolder.toLocalFile()
            if (dir.length > 0)
                app.cacheDirectory = dir
        }
    }

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: Math.min(parent.width, 720)
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: Theme.pagePadding
            anchors.bottomMargin: Theme.pagePadding
            spacing: Theme.sectionGap

            // ── 页头 ──
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: qsTr("设置")
                    font.pixelSize: 24
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: qsTr("管理缓存、存储空间与播放偏好")
                    font.pixelSize: 13
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            // ── 缓存存储 ──
            SettingsCard {
                Layout.fillWidth: true
                title: qsTr("缓存与存储")
                subtitle: qsTr("在线音乐会先下载到本地再播放；已缓存的歌曲下次可更快播放")

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: qsTr("缓存目录")
                            font.pixelSize: 13
                            font.bold: true
                            color: Theme.textPrimary
                        }
                        Text {
                            text: qsTr("默认位于程序目录下的 cache 文件夹")
                            font.pixelSize: 12
                            color: Theme.textTertiary
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
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
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    verticalAlignment: Text.AlignVCenter
                                    text: app.cacheDirectory
                                    font.pixelSize: 12
                                    color: Theme.textSecondary
                                    elide: Text.ElideMiddle
                                }
                            }

                            ToolButton {
                                text: qsTr("浏览…")
                                font.pixelSize: 13
                                implicitHeight: 40
                                implicitWidth: Math.max(72, implicitContentWidth + 24)
                                onClicked: cacheFolderDialog.open()
                                background: Rectangle {
                                    radius: Theme.radiusMd
                                    color: parent.down ? Theme.accentSoft
                                         : (parent.hovered ? Theme.bgHover : Theme.bgCard)
                                    border.color: Theme.borderStrong
                                }
                                contentItem: Text {
                                    text: parent.text
                                    font: parent.font
                                    color: Theme.textPrimary
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Theme.border
                    }

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

                        Text {
                            text: qsTr("超出上限时，将按最久未使用顺序自动清理旧文件")
                            font.pixelSize: 12
                            color: Theme.textTertiary
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Theme.border
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: qsTr("已用空间")
                                font.pixelSize: 13
                                font.bold: true
                                color: Theme.textPrimary
                            }
                            Text {
                                text: app.cacheUsedText + " / " + app.cacheMaxSizeMb + " MB"
                                font.pixelSize: 12
                                color: Theme.textSecondary
                            }
                        }

                        ToolButton {
                            text: qsTr("清空缓存")
                            implicitHeight: 36
                            implicitWidth: Math.max(88, implicitContentWidth + 24)
                            onClicked: clearCacheDialog.open()
                            background: Rectangle {
                                radius: Theme.radiusMd
                                color: parent.down ? "#1AEF4444"
                                     : (parent.hovered ? "#0DEF4444" : Theme.bgCard)
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

            // ── 播放说明 ──
            SettingsCard {
                Layout.fillWidth: true
                title: qsTr("播放与缓存说明")
                subtitle: qsTr("了解为什么有的歌曲需要等待")

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    SettingsHintRow {
                        title: qsTr("立即播放")
                        body: qsTr("该歌曲音频已缓存在本地，或来自本地音乐库，无需再次下载。")
                    }
                    SettingsHintRow {
                        title: qsTr("正在缓冲")
                        body: qsTr("在线歌曲需先解析播放地址并下载到缓存目录（CDN 防盗链限制），完成后自动播放。")
                    }
                    SettingsHintRow {
                        title: qsTr("专辑封面")
                        body: qsTr("加入「我喜欢的音乐」后，封面会保存到缓存目录，下次打开仍可显示。")
                    }
                }
            }

            Item { Layout.preferredHeight: 8 }
        }
    }

    Dialog {
        id: clearCacheDialog
        title: qsTr("清空缓存")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: app.clearMediaCache()

        Label {
            text: qsTr("将删除所有已缓存的音频与封面文件，不影响歌单列表。确定继续？")
            wrapMode: Text.WordWrap
            width: 320
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

    component SettingsCard: Rectangle {
        id: card
        property string title
        property string subtitle
        default property alias content: cardContent.data

        radius: Theme.radiusLg
        color: Theme.bgCard
        border.color: Theme.borderStrong
        implicitHeight: cardLayout.implicitHeight + 32

        ColumnLayout {
            id: cardLayout
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Text {
                    text: card.title
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    visible: card.subtitle.length > 0
                    text: card.subtitle
                    font.pixelSize: 12
                    color: Theme.textTertiary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }

            Item {
                id: cardContent
                Layout.fillWidth: true
                implicitHeight: childrenRect.height
            }
        }
    }

    component SettingsHintRow: RowLayout {
        property string title
        property string body
        Layout.fillWidth: true
        spacing: 10

        Rectangle {
            Layout.preferredWidth: 6
            Layout.preferredHeight: 6
            Layout.topMargin: 6
            radius: 3
            color: Theme.accent
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Text {
                text: title
                font.pixelSize: 13
                font.bold: true
                color: Theme.textPrimary
            }
            Text {
                text: body
                font.pixelSize: 12
                color: Theme.textSecondary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }
}
