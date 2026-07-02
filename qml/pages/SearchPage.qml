// SearchPage.qml — 在线音乐搜索（MyFreeMp3）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    readonly property var hotKeywords: [
        qsTr("林俊杰"), qsTr("周杰伦"), qsTr("邓紫棋"),
        qsTr("薛之谦"), qsTr("陈奕迅"), qsTr("Beyond")
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: Theme.sectionGap

        // ── Hero 区：搜索焦点 + 状态反馈 ──
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.searchHeroHeight
            radius: Theme.radiusLg
            clip: true

            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0; color: Theme.searchHeroStart }
                GradientStop { position: 1; color: Theme.searchHeroEnd }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 4
                color: Theme.accent
                radius: 2
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 8

                Text {
                    text: qsTr("探索在线曲库")
                    font.pixelSize: 24
                    font.bold: true
                    color: Theme.textPrimary
                }

                Text {
                    text: app.searchKeyword.length > 0
                          ? qsTr("正在浏览「%1」的搜索结果").arg(app.searchKeyword)
                          : qsTr("在顶部搜索框输入歌名或歌手，或点击热门标签")
                    font.pixelSize: 13
                    color: Theme.textSecondary
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    BusyIndicator {
                        visible: app.searchBusy
                        running: app.searchBusy
                        Layout.preferredWidth: 18
                        Layout.preferredHeight: 18
                    }

                    Text {
                        Layout.fillWidth: true
                        text: app.searchStatus
                        font.pixelSize: 13
                        color: app.searchBusy ? Theme.accent : Theme.textSecondary
                        elide: Text.ElideRight
                    }
                }
            }
        }

        // ── 热门搜索 Chip（降低搜索摩擦）──
        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: root.hotKeywords
                delegate: Button {
                    required property string modelData
                    text: modelData
                    enabled: !app.searchBusy
                    implicitHeight: Theme.searchChipHeight
                    padding: 12
                    onClicked: {
                        app.searchOnline(modelData, 1)
                        app.currentPage = 2
                    }
                    background: Rectangle {
                        radius: Theme.searchChipHeight / 2
                        color: parent.down ? Theme.accentSoft
                             : (parent.hovered ? Theme.bgHover : Theme.bgCard)
                        border.color: parent.hovered ? Theme.accent : Theme.border
                        Behavior on border.color { ColorAnimation { duration: 200 } }
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }
                    contentItem: Text {
                        text: parent.text
                        color: parent.hovered ? Theme.accent : Theme.textSecondary
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        // ── 结果列表 ──
        SongList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.searchResultCount > 0
            model: app.searchResultModel
            onRowActivated: function(row) { app.playSearchRow(row) }
        }

        // ── 空状态（ui-ux-pro-max：引导而非留白）──
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: app.searchResultCount === 0 && !app.searchBusy

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width, 360)
                spacing: 12

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 56
                    height: 56
                    radius: 28
                    color: Theme.accentSoft
                    AppIcon {
                        anchors.centerIn: parent
                        name: Icons.playlist
                        size: 24
                        color: Theme.accent
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: app.searchKeyword.length > 0
                          ? qsTr("暂无结果，换个关键词试试")
                          : qsTr("搜索 MyFreeMp3 在线曲库")
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.textPrimary
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("支持分页浏览，点击歌曲即可在线播放")
                    font.pixelSize: 13
                    color: Theme.textTertiary
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }

        // ── 底部分页 ──
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: app.searchResultCount > 0 || app.searchCurrentPage > 1

            Button {
                text: qsTr("上一页")
                enabled: app.searchHasPrevious && !app.searchBusy
                implicitHeight: 36
                Layout.preferredWidth: 96
                onClicked: app.searchPreviousPage()
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: parent.enabled
                           ? (parent.down ? Theme.accentSoft
                              : (parent.hovered ? Theme.bgHover : Theme.bgCard))
                           : "transparent"
                    border.color: parent.enabled && parent.hovered ? Theme.accent : Theme.border
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? Theme.textPrimary : Theme.textTertiary
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: qsTr("第 %1 页").arg(app.searchCurrentPage)
                font.pixelSize: 13
                color: Theme.textSecondary
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("下一页")
                enabled: app.searchHasNext && !app.searchBusy
                implicitHeight: 36
                Layout.preferredWidth: 96
                onClicked: app.searchNextPage()
                background: Rectangle {
                    radius: Theme.radiusMd
                    color: parent.enabled
                           ? (parent.down ? Theme.accentSoft
                              : (parent.hovered ? Theme.bgHover : Theme.bgCard))
                           : "transparent"
                    border.color: parent.enabled && parent.hovered ? Theme.accent : Theme.border
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
                contentItem: Text {
                    text: parent.text
                    color: parent.enabled ? Theme.textPrimary : Theme.textTertiary
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
