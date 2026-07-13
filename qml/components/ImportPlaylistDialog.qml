// ImportPlaylistDialog.qml — 网易云 / QQ 音乐歌单链接导入
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

CardDialog {
    id: root

    parent: Overlay.overlay
    heading: qsTr("导入外部歌单")
    description: ""
    primaryText: qsTr("导入匹配歌曲")
    secondaryText: qsTr("关闭")
    closeOnPrimary: false

    property string initialPlaylistId: ""
    property string resultText: ""

    function openForPlaylist(playlistId) {
        initialPlaylistId = playlistId || ""
        resultText = ""
        linkField.text = ""
        newPlaylistField.text = ""
        app.clearPlaylistImportPreview()
        reloadTargets()
        open()
        linkField.forceActiveFocus()
    }

    function reloadTargets() {
        targetModel.clear()
        const playlists = app.sidebarPlaylists
        let selected = 0
        for (let i = 0; i < playlists.length; ++i) {
            targetModel.append({
                playlistId: playlists[i].id,
                name: playlists[i].name,
                trackCount: playlists[i].trackCount,
                createNew: false
            })
            if (playlists[i].id === initialPlaylistId)
                selected = i
        }
        targetModel.append({
            playlistId: "",
            name: qsTr("新建歌单…"),
            trackCount: 0,
            createNew: true
        })
        targetCombo.currentIndex = selected
    }

    function selectedTargetId() {
        if (targetCombo.currentIndex < 0 || targetCombo.currentIndex >= targetModel.count)
            return ""
        const item = targetModel.get(targetCombo.currentIndex)
        return item.createNew ? "" : item.playlistId
    }

    function creatingNewPlaylist() {
        return targetCombo.currentIndex >= 0
            && targetCombo.currentIndex < targetModel.count
            && targetModel.get(targetCombo.currentIndex).createNew
    }

    function previewLink() {
        resultText = ""
        app.previewExternalPlaylist(linkField.text.trim())
    }

    onOpened: reloadTargets()
    onRejected: {
        if (app.playlistImportBusy)
            app.cancelPlaylistImport()
        close()
    }
    onAccepted: {
        if (app.playlistImportBusy || app.playlistImportMatchedCount <= 0)
            return
        const summary = app.confirmPlaylistImport(
            selectedTargetId(),
            creatingNewPlaylist() ? newPlaylistField.text.trim() : "")
        resultText = summary.message || ""
    }

    ListModel { id: targetModel }

    Item {
        Layout.preferredWidth: 760
        Layout.preferredHeight: 520

        ColumnLayout {
            anchors.fill: parent
            spacing: 14

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 86
                radius: 18
                color: Theme.accentSoft
                border.color: Qt.rgba(0, 0.47, 0.83, 0.22)

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14

                    Rectangle {
                        Layout.preferredWidth: 48
                        Layout.preferredHeight: 48
                        radius: 16
                        color: Theme.bgCard
                        border.color: Theme.cardBorder
                        AppIcon {
                            anchors.centerIn: parent
                            name: Icons.playlist
                            size: 24
                            color: Theme.accent
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4
                        Text {
                            Layout.fillWidth: true
                            text: qsTr("支持网易云音乐 / QQ 音乐公开歌单")
                            color: Theme.textPrimary
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 112
                        Layout.preferredHeight: 30
                        radius: 15
                        color: Theme.bgCard
                        border.color: Theme.cardBorder
                        Text {
                            anchors.centerIn: parent
                            text: app.playlistImportPlatform.length > 0 ? app.playlistImportPlatform : qsTr("自动识别")
                            color: Theme.accent
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                TextField {
                    id: linkField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 38
                    placeholderText: qsTr("粘贴歌单分享链接，例如 https://music.163.com/#/playlist?id=...")
                    selectByMouse: true
                    color: Theme.fieldText
                    placeholderTextColor: Theme.fieldPlaceholder
                    enabled: !app.playlistImportBusy
                    onAccepted: root.previewLink()
                    background: Rectangle {
                        radius: 19
                        color: Theme.fieldBg
                        border.color: linkField.activeFocus ? Theme.accent : Theme.borderStrong
                    }
                }

                Button {
                    Layout.preferredWidth: 108
                    Layout.preferredHeight: 38
                    text: app.playlistImportBusy ? qsTr("匹配中") : qsTr("预览")
                    enabled: !app.playlistImportBusy
                    onClicked: root.previewLink()
                    background: Rectangle {
                        radius: 19
                        color: parent.enabled ? (parent.hovered ? "#1689E8" : Theme.accent) : Theme.borderStrong
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                ComboBox {
                    id: targetCombo
                    Layout.preferredWidth: 240
                    Layout.preferredHeight: 36
                    model: targetModel
                    textRole: "name"
                    enabled: !app.playlistImportBusy
                    background: Rectangle {
                        radius: 18
                        color: Theme.fieldBg
                        border.color: targetCombo.activeFocus || targetCombo.popup.visible ? Theme.accent : Theme.borderStrong
                    }
                }

                TextField {
                    id: newPlaylistField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    visible: root.creatingNewPlaylist()
                    placeholderText: qsTr("新歌单名称")
                    color: Theme.fieldText
                    placeholderTextColor: Theme.fieldPlaceholder
                    enabled: !app.playlistImportBusy
                    background: Rectangle {
                        radius: 18
                        color: Theme.fieldBg
                        border.color: newPlaylistField.activeFocus ? Theme.accent : Theme.borderStrong
                    }
                }

                Item { Layout.fillWidth: true; visible: !root.creatingNewPlaylist() }

                Text {
                    text: app.playlistImportTotalCount > 0
                          ? qsTr("%1/%2 · 匹配 %3 · 未匹配 %4")
                                .arg(app.playlistImportProcessedCount)
                                .arg(app.playlistImportTotalCount)
                                .arg(app.playlistImportMatchedCount)
                                .arg(app.playlistImportUnmatchedCount)
                          : qsTr("等待预览")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 18
                color: Qt.rgba(1, 1, 1, 0.58)
                border.color: Theme.cardBorder
                clip: true

                ListView {
                    id: previewList
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                    model: app.playlistImportModel
                    clip: true
                    visible: !app.playlistImportBusy && count > 0

                    delegate: Rectangle {
                        required property string externalTitle
                        required property string externalArtist
                        required property string matchedTitle
                        required property string matchedArtist
                        required property string sourceLabel
                        required property string statusText
                        required property string statusKind
                        required property bool hasMatch

                        width: previewList.width
                        height: 58
                        radius: 14
                        color: statusKind === "missing" ? "#0AEF4444" : Theme.bgCard
                        border.color: statusKind === "missing" ? "#22EF4444"
                                     : (statusKind === "imported" ? "#3322C55E" : Theme.cardBorder)

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 3
                                Text {
                                    Layout.fillWidth: true
                                    text: externalTitle + (externalArtist.length > 0 ? " - " + externalArtist : "")
                                    color: Theme.textPrimary
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: hasMatch
                                          ? qsTr("匹配：%1 - %2 · %3").arg(matchedTitle).arg(matchedArtist).arg(sourceLabel)
                                          : qsTr("暂未在当前在线音源中找到可播放版本")
                                    color: hasMatch ? Theme.textSecondary : "#EF4444"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                            }

                            Rectangle {
                                Layout.preferredWidth: 72
                                Layout.preferredHeight: 26
                                radius: 13
                                color: statusKind === "missing" ? "#14EF4444"
                                     : (statusKind === "duplicate" ? "#14F59E0B" : "#1422C55E")
                                border.color: statusKind === "missing" ? "#55EF4444"
                                             : (statusKind === "duplicate" ? "#55F59E0B" : "#5522C55E")
                                Text {
                                    anchors.centerIn: parent
                                    text: statusText
                                    color: statusKind === "missing" ? "#EF4444"
                                         : (statusKind === "duplicate" ? "#B45309" : "#16A34A")
                                    font.pixelSize: 12
                                    font.weight: Font.DemiBold
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    anchors.centerIn: parent
                    width: parent.width - 80
                    spacing: 12
                    visible: app.playlistImportBusy

                    BusyIndicator {
                        Layout.alignment: Qt.AlignHCenter
                        running: app.playlistImportBusy
                        implicitWidth: 42
                        implicitHeight: 42
                    }

                    Text {
                        Layout.fillWidth: true
                        text: app.playlistImportStatus
                        color: Theme.textSecondary
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }

                }

                Text {
                    anchors.centerIn: parent
                    visible: !app.playlistImportBusy && previewList.count === 0 && app.playlistImportTotalCount > 0
                    text: app.playlistImportStatus
                    color: Theme.textTertiary
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width - 80
                    wrapMode: Text.WordWrap
                }
            }

            Text {
                Layout.fillWidth: true
                visible: resultText.length > 0 || app.playlistImportTotalCount > 0 || app.playlistImportBusy
                text: resultText.length > 0 ? resultText : app.playlistImportStatus
                color: resultText.length > 0 ? Theme.accent : Theme.textSecondary
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }
    }
}
