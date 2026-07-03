// LocalMusicPage.qml — 网易云风格本地音乐
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root
    property var trackModel
    signal importClicked()

    readonly property var sortTabs: [
        qsTr("默认"), qsTr("歌手"), qsTr("专辑"), qsTr("文件夹")
    ]

    property int activeSortTab: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: 16

        // ── 标题行 ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            RowLayout {
                spacing: 8
                Text {
                    text: qsTr("本地音乐")
                    font.pixelSize: 26
                    font.bold: true
                    color: Theme.textPrimary
                }
                Text {
                    text: app.trackCount > 0 ? qsTr("共 %1 首").arg(app.trackCount) : ""
                    font.pixelSize: 14
                    color: Theme.textSecondary
                    visible: app.trackCount > 0
                }
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("选择目录 >")
                flat: true
                onClicked: root.importClicked()
                contentItem: Text {
                    text: parent.text
                    color: Theme.textSecondary
                    font.pixelSize: 13
                }
                background: Rectangle {
                    color: parent.hovered ? Theme.bgHover : "transparent"
                    radius: Theme.radiusMd
                }
            }
        }

        // ── 工具栏：播放全部 / 随机 / 更多 ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: qsTr("  播放全部")
                enabled: app.trackCount > 0
                implicitHeight: 32
                implicitWidth: 108
                onClicked: app.playAllLocal()
                background: Rectangle {
                    radius: 16
                    color: parent.enabled
                           ? (parent.down ? "#006CBD" : (parent.hovered ? "#1088E0" : Theme.accent))
                           : Theme.sliderTrack
                }
                contentItem: RowLayout {
                    spacing: 4
                    AppIcon { name: Icons.play; size: 12; color: "#FFFFFF" }
                    Text {
                        text: parent.parent.text.trim()
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.bold: true
                    }
                }
            }

            ToolButton {
                implicitWidth: 32
                implicitHeight: 32
                enabled: app.trackCount > 0
                ToolTip.text: app.playbackModeTooltip
                onClicked: app.cyclePlaybackMode()
                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? Theme.bgHover : Theme.bgCard
                    border.color: Theme.borderStrong
                }
                contentItem: AppIcon {
                    name: Icons.modeIconName(app.playbackMode)
                    size: 16
                    color: Theme.textSecondary
                    anchors.centerIn: parent
                }
            }

            ToolButton {
                implicitWidth: 32
                implicitHeight: 32
                enabled: false
                opacity: 0.4
                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? Theme.bgHover : Theme.bgCard
                    border.color: Theme.borderStrong
                }
                contentItem: Text {
                    text: "···"
                    font.pixelSize: 14
                    font.bold: true
                    color: Theme.textSecondary
                    anchors.centerIn: parent
                }
            }

            Item { Layout.fillWidth: true }

            // ── 搜索 + 排序 Tab ──
            Rectangle {
                Layout.preferredWidth: 200
                Layout.preferredHeight: 32
                radius: 16
                color: Theme.bgCard
                border.color: localSearch.activeFocus ? Theme.accent : Theme.borderStrong

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8
                    spacing: 6
                    AppIcon { name: Icons.search; size: 14; color: Theme.textTertiary }
                    TextField {
                        id: localSearch
                        Layout.fillWidth: true
                        placeholderText: qsTr("搜索本地歌曲")
                        font.pixelSize: 13
                        color: Theme.textPrimary
                        placeholderTextColor: Theme.textTertiary
                        background: Item {}
                    }
                }
            }

            Row {
                spacing: 16
                Repeater {
                    model: root.sortTabs
                    delegate: Text {
                        required property int index
                        required property string modelData
                        text: modelData
                        font.pixelSize: 13
                        color: root.activeSortTab === index ? Theme.textPrimary : Theme.textSecondary
                        font.weight: root.activeSortTab === index ? Font.DemiBold : Font.Normal
                        opacity: index === 0 ? 1.0 : 0.45
                        MouseArea {
                            anchors.fill: parent
                            enabled: index === 0
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: root.activeSortTab = index
                        }
                    }
                }
            }
        }

        LocalMusicList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.trackCount > 0
            model: root.trackModel
            onRowSelected: function(row) { app.selectLocalRow(row) }
            onRowActivated: function(row) { app.playRow(row) }
            onLikeClicked: function(row) { app.toggleLikeLocalRow(row) }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.trackCount === 0

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width, 360)
                spacing: 12

                Text {
                    Layout.fillWidth: true
                    text: qsTr("暂无本地歌曲")
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("点击右上角「选择目录」导入音频文件")
                    font.pixelSize: 13
                    color: Theme.textTertiary
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }

                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("导入音乐")
                    onClicked: root.importClicked()
                    background: Rectangle {
                        radius: 16
                        color: parent.down ? "#006CBD" : Theme.accent
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
