// SearchPage.qml — 网易云风格搜索结果页（顶栏搜索后在此展示）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    readonly property var categoryTabs: [
        qsTr("综合"), qsTr("单曲"), qsTr("歌单"), qsTr("播客"),
        qsTr("专辑"), qsTr("歌手"), qsTr("MV"), qsTr("用户"),
        qsTr("歌词"), qsTr("声音")
    ]

    property int activeTab: 1

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: root.width
            spacing: 0

            // ── 搜索关键词标题 ──
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: app.searchKeyword.length > 0 ? 72 : 0
                visible: app.searchKeyword.length > 0

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.pagePadding
                    anchors.verticalCenter: parent.verticalCenter
                    text: app.searchKeyword
                    font.pixelSize: 26
                    font.bold: true
                    color: Theme.textPrimary
                }
            }

            // ── 分类 Tab（网易云布局，当前仅「单曲」可用）──
            Row {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.preferredHeight: Theme.contentTabHeight
                spacing: 24

                Repeater {
                    model: root.categoryTabs
                    delegate: Item {
                        id: tabItem
                        required property int index
                        required property string modelData
                        property bool isActive: root.activeTab === index

                        implicitWidth: tabLabel.implicitWidth
                        implicitHeight: Theme.contentTabHeight

                        Text {
                            id: tabLabel
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            font.pixelSize: 14
                            font.weight: isActive ? Font.DemiBold : Font.Normal
                            color: isActive ? Theme.textPrimary : Theme.textSecondary
                            opacity: index === 1 ? 1.0 : 0.45
                        }

                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: tabLabel.width
                            height: 2
                            radius: 1
                            color: Theme.accent
                            visible: isActive
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: index === 1
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: root.activeTab = index
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            // ── 工具栏：播放全部 / 收藏全部 + 状态 ──
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.topMargin: 16
                Layout.bottomMargin: 8
                spacing: 12
                visible: app.searchResultCount > 0 || app.searchBusy

                Button {
                    text: qsTr("  播放全部")
                    enabled: app.searchResultCount > 0 && !app.searchBusy
                    implicitHeight: 32
                    implicitWidth: 108
                    onClicked: app.playAllSearchResults()
                    background: Rectangle {
                        radius: 16
                        color: parent.enabled
                               ? (parent.down ? "#006CBD" : (parent.hovered ? "#1088E0" : Theme.accent))
                               : Theme.sliderTrack
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }
                    contentItem: RowLayout {
                        spacing: 4
                        AppIcon {
                            name: Icons.play
                            size: 12
                            color: "#FFFFFF"
                        }
                        Text {
                            text: parent.parent.text.trim()
                            color: "#FFFFFF"
                            font.pixelSize: 13
                            font.bold: true
                        }
                    }
                }

                Button {
                    text: qsTr("  收藏全部")
                    enabled: app.searchResultCount > 0 && !app.searchBusy
                    implicitHeight: 32
                    implicitWidth: 108
                    onClicked: app.likeAllSearchResults()
                    background: Rectangle {
                        radius: 16
                        color: parent.enabled
                               ? (parent.down ? Theme.accentSoft
                                  : (parent.hovered ? Theme.bgHover : Theme.bgCard))
                               : "transparent"
                        border.color: parent.enabled ? Theme.borderStrong : Theme.border
                    }
                    contentItem: RowLayout {
                        spacing: 4
                        AppIcon {
                            name: Icons.folderPlus
                            size: 12
                            color: Theme.textSecondary
                        }
                        Text {
                            text: parent.parent.text.trim()
                            color: Theme.textSecondary
                            font.pixelSize: 13
                        }
                    }
                }

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
                    color: app.searchBusy ? Theme.accent : Theme.textTertiary
                    elide: Text.ElideRight
                }
            }

            // ── 搜索结果列表 ──
            SearchResultList {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(200, app.searchResultCount * Theme.searchRowHeight + Theme.searchTableHeaderHeight)
                visible: app.searchResultCount > 0
                model: app.searchResultModel
                onRowSelected: function(row) { app.selectSearchRow(row) }
                onRowActivated: function(row) { app.playSearchRow(row) }
                onLikeClicked: function(row) { app.toggleLikeSearchRow(row) }
                onAddToPlaylist: function(row, playlistId) { app.addSearchRowToPlaylist(row, playlistId) }
            }

            // ── 空状态 ──
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 280
                visible: app.searchResultCount === 0 && !app.searchBusy

                ColumnLayout {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - Theme.pagePadding * 2, 360)
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
                              : qsTr("在顶部搜索框输入歌名或歌手")
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("支持分页浏览 · 右键可播放或加入歌单")
                        font.pixelSize: 13
                        color: Theme.textTertiary
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }

            // ── 分页 ──
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.topMargin: 8
                Layout.bottomMargin: Theme.pagePadding
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
}
