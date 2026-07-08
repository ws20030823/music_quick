// FeaturedPlaylistPage.qml — 精选歌单详情（歌曲宝搜索结果）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: root.width
            spacing: 0

            // ── 歌单头部 ──
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.topMargin: Theme.pagePadding
                Layout.bottomMargin: 8
                spacing: 20

                Rectangle {
                    Layout.preferredWidth: 160
                    Layout.preferredHeight: 160
                    radius: Theme.cardCoverRadius
                    clip: true
                    color: Theme.bgCard
                    border.color: Theme.borderStrong
                    border.width: 1

                    Image {
                        anchors.fill: parent
                        source: app.activeFeaturedPlaylistCoverUrl
                        fillMode: Image.PreserveAspectCrop
                        smooth: true
                        asynchronous: true
                    }

                    Rectangle {
                        anchors.fill: parent
                        visible: app.activeFeaturedPlaylistCoverUrl.length === 0
                        color: Theme.accentSoft
                        Text {
                            anchors.centerIn: parent
                            text: app.activeFeaturedPlaylistTitle.length > 0
                                  ? app.activeFeaturedPlaylistTitle.charAt(0) : "?"
                            font.pixelSize: 48
                            font.bold: true
                            color: Theme.accent
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: app.activeFeaturedPlaylistTitle
                        font.pixelSize: 26
                        font.bold: true
                        color: Theme.textPrimary
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        text: app.activeFeaturedPlaylistSubtitle
                        font.pixelSize: 14
                        color: Theme.textSecondary
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("来源：歌曲宝")
                        font.pixelSize: 13
                        color: Theme.textTertiary
                    }

                    RowLayout {
                        spacing: 12
                        Layout.topMargin: 4

                        Button {
                            text: qsTr("  播放全部")
                            enabled: app.searchResultCount > 0 && !app.searchBusy
                            implicitHeight: 36
                            implicitWidth: 120
                            onClicked: app.playAllSearchResults()
                            background: Rectangle {
                                radius: 18
                                color: parent.enabled
                                       ? (parent.down ? "#006CBD" : (parent.hovered ? "#1088E0" : Theme.accent))
                                       : Theme.sliderTrack
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
                            id: favoriteBtn
                            implicitHeight: 36
                            implicitWidth: 120
                            onClicked: app.toggleFavoriteFeaturedPlaylist()
                            background: Rectangle {
                                radius: 18
                                color: parent.down ? Theme.accentSoft
                                     : (parent.hovered ? Theme.bgHover : Theme.bgCard)
                                border.color: app.activeFeaturedPlaylistFavorited ? Theme.accent : Theme.borderStrong
                            }
                            contentItem: RowLayout {
                                spacing: 4
                                AppIcon {
                                    name: app.activeFeaturedPlaylistFavorited ? Icons.heartFilled : Icons.heart
                                    size: 12
                                    color: app.activeFeaturedPlaylistFavorited ? Theme.accent : Theme.textSecondary
                                }
                                Text {
                                    text: app.activeFeaturedPlaylistFavorited ? qsTr("已收藏") : qsTr("收藏歌单")
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
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            SearchResultList {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(200, app.searchResultCount * Theme.searchRowHeight + Theme.searchTableHeaderHeight)
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                visible: app.searchResultCount > 0
                model: app.searchResultModel
                onRowSelected: function(row) { app.selectSearchRow(row) }
                onRowActivated: function(row) { app.playSearchRow(row) }
                onLikeClicked: function(row) { app.toggleLikeSearchRow(row) }
                onAddToPlaylist: function(row, playlistId) { app.addSearchRowToPlaylist(row, playlistId) }
            }

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
                        text: qsTr("暂无歌曲，请稍后再试")
                        font.pixelSize: 16
                        font.bold: true
                        color: Theme.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("歌单内容来自歌曲宝搜索")
                        font.pixelSize: 13
                        color: Theme.textTertiary
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }
                }
            }

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
