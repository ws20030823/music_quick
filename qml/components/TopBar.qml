// TopBar.qml — 右侧顶栏：后退/搜索/窗口控制（Logo 已移至 AppSidebar）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import MusicQuick

Rectangle {
    id: root
    color: Theme.topBarBg
    implicitHeight: Theme.topBarHeight

    required property Window window

    property alias searchField: searchInput

    signal searchSubmitted(string keyword)

    ListModel {
        id: sourceListModel
    }

    function reloadSourceList() {
        sourceListModel.clear()
        const sources = app.musicSources
        for (let i = 0; i < sources.length; ++i) {
            sourceListModel.append({
                sourceId: sources[i].sourceId,
                name: sources[i].name
            })
        }
        syncSourceSelection()
    }

    function syncSourceSelection() {
        for (let i = 0; i < sourceListModel.count; ++i) {
            if (sourceListModel.get(i).sourceId === app.activeMusicSourceId) {
                sourceCombo.currentIndex = i
                return
            }
        }
    }

    function sourceLabelAt(index) {
        if (index < 0 || index >= sourceListModel.count) {
            return app.activeMusicSourceName || qsTr("来源")
        }
        return sourceListModel.get(index).name
    }

    Component.onCompleted: reloadSourceList()

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: Theme.border
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 0
        spacing: 10

        ToolButton {
            implicitWidth: 32
            implicitHeight: 32
            enabled: app.canNavigateBack
            opacity: enabled ? 1 : 0.4
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("返回")
            onClicked: app.navigateBack()
            background: Rectangle {
                radius: 8
                color: parent.hovered ? Theme.bgHover : Theme.bgCard
                border.color: Theme.borderStrong
            }
            contentItem: AppIcon {
                name: Icons.chevronLeft
                size: 16
                color: Theme.textSecondary
                anchors.centerIn: parent
            }
        }

        ComboBox {
            id: sourceCombo
            Layout.preferredWidth: Math.max(112, implicitContentWidth + 36)
            Layout.preferredHeight: Theme.searchChipHeight
            Layout.alignment: Qt.AlignVCenter
            model: sourceListModel
            textRole: "name"
            valueRole: "sourceId"
            hoverEnabled: true

            onActivated: function(index) {
                const sourceId = sourceCombo.currentValue
                if (sourceId && sourceId !== app.activeMusicSourceId)
                    app.activeMusicSourceId = sourceId
            }

            onCurrentIndexChanged: {
                if (sourceCombo.currentIndex < 0)
                    return
                const sourceId = sourceCombo.currentValue
                if (sourceId && sourceId !== app.activeMusicSourceId)
                    app.activeMusicSourceId = sourceId
            }

            Connections {
                target: app
                function onActiveMusicSourceChanged() {
                    root.syncSourceSelection()
                }
            }

            contentItem: Text {
                leftPadding: 12
                rightPadding: sourceCombo.indicator.width + sourceCombo.spacing
                text: root.sourceLabelAt(sourceCombo.currentIndex)
                font.pixelSize: 13
                color: Theme.textPrimary
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            indicator: Text {
                x: sourceCombo.width - width - 10
                y: (sourceCombo.height - height) / 2
                text: "\u25BE"
                font.pixelSize: 11
                color: Theme.textSecondary
            }

            background: Rectangle {
                implicitWidth: 112
                implicitHeight: Theme.searchChipHeight
                radius: Theme.searchChipHeight / 2
                color: sourceCombo.down ? Theme.accentSoft
                       : (sourceCombo.hovered ? Theme.bgHover : Theme.bgCard)
                border.color: sourceCombo.activeFocus || sourceCombo.popup.visible
                              ? Theme.accent : Theme.borderStrong
                Behavior on border.color { ColorAnimation { duration: 200 } }
            }

            delegate: ItemDelegate {
                width: sourceCombo.width
                required property string name
                text: name
                font.pixelSize: 13
                highlighted: sourceCombo.highlightedIndex === index
                background: Rectangle {
                    color: highlighted ? Theme.accentSoft
                           : (parent.hovered ? Theme.bgHover : "transparent")
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.maximumWidth: 480
            Layout.preferredHeight: 34
            Layout.alignment: Qt.AlignVCenter
            radius: 17
            color: Theme.bgCard
            border.color: searchInput.activeFocus ? Theme.accent : Theme.borderStrong

            Behavior on border.color { ColorAnimation { duration: 200 } }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 10
                spacing: 8

                AppIcon {
                    name: Icons.search
                    size: 15
                    color: Theme.textTertiary
                }

                TextField {
                    id: searchInput
                    Layout.fillWidth: true
                    placeholderText: app.searchKeyword.length > 0
                                         ? app.searchKeyword
                                         : qsTr("搜索音乐、歌手")
                    font.pixelSize: 13
                    color: Theme.textPrimary
                    placeholderTextColor: Theme.textTertiary
                    selectByMouse: true
                    background: Item {}
                    onAccepted: root.searchSubmitted(text.trim())
                }
            }
        }

        ToolButton {
            implicitWidth: 32
            implicitHeight: 32
            enabled: false
            opacity: 0.35
            Layout.alignment: Qt.AlignVCenter
            background: Rectangle {
                radius: 8
                color: parent.hovered ? Theme.bgHover : Theme.bgCard
                border.color: Theme.borderStrong
            }
            contentItem: AppIcon {
                name: Icons.mic
                size: 16
                color: Theme.textSecondary
                anchors.centerIn: parent
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            MouseArea {
                anchors.fill: parent
                onPressed: function(mouse) {
                    if (mouse.button === Qt.LeftButton && root.window)
                        root.window.startSystemMove()
                }
            }
        }

        ToolButton {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.rightMargin: 4
            implicitWidth: 32
            implicitHeight: 32
            ToolTip.text: qsTr("设置")
            onClicked: app.toggleSettings()
            background: Rectangle {
                radius: 8
                color: app.settingsVisible ? Theme.accentSoft
                     : (parent.hovered ? Theme.bgHover : "transparent")
            }
            contentItem: AppIcon {
                name: Icons.settings
                size: 16
                color: app.settingsVisible ? Theme.accent : Theme.textSecondary
                anchors.centerIn: parent
            }
        }

        WindowControls {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            window: root.window
        }
    }
}
