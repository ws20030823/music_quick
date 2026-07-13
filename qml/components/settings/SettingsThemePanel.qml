// SettingsThemePanel.qml — 换肤
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import MusicQuick

SettingsPanelLayout {
    title: qsTr("换肤")
    subtitle: qsTr("自定义壁纸、壁纸透明度与卡片蒙层浓度")

    FileDialog {
        id: wallpaperDialog
        title: qsTr("选择背景壁纸")
        nameFilters: [qsTr("图片 (*.jpg *.jpeg *.png *.webp)"), qsTr("所有文件 (*.*)")]
        onAccepted: {
            const url = selectedFile.toString().length > 0 ? selectedFile : currentFile
            if (url.toString().length > 0)
                app.homeWallpaperPath = url.toString()
        }
    }

    SettingsCard {
        title: qsTr("背景壁纸")
        subtitle: qsTr("为整个播放器窗口设置背景图")

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                radius: Theme.radiusMd
                color: Theme.bgCard
                border.color: Theme.borderStrong
                clip: true

                Image {
                    anchors.fill: parent
                    source: app.homeWallpaperUrl
                    fillMode: Image.PreserveAspectCrop
                    visible: status === Image.Ready
                    asynchronous: true
                    cache: false
                }

                Text {
                    anchors.centerIn: parent
                    visible: !app.hasHomeWallpaper
                    text: qsTr("未设置壁纸")
                    font.pixelSize: 13
                    color: Theme.textTertiary
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                ToolButton {
                    text: qsTr("选择图片")
                    implicitHeight: 36
                    onClicked: wallpaperDialog.open()
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.accentSoft : Theme.bgCard
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

                ToolButton {
                    text: qsTr("清除")
                    implicitHeight: 36
                    enabled: app.hasHomeWallpaper
                    onClicked: app.homeWallpaperPath = ""
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.bgHover : Theme.bgCard
                        border.color: Theme.borderStrong
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 13
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    SettingsCard {
        title: qsTr("壁纸不透明度")
        subtitle: qsTr("仅控制背景白底或壁纸；与卡片蒙层无关")

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: qsTr("壁纸不透明度")
                    font.pixelSize: 13
                    font.bold: true
                    color: Theme.textPrimary
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: Math.round(app.uiBackgroundOpacity * 100) + "%"
                    font.pixelSize: 13
                    color: Theme.accent
                    font.bold: true
                }
            }

            Slider {
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: app.uiBackgroundOpacity * 100
                onMoved: app.uiBackgroundOpacity = value / 100
                onPressedChanged: {
                    if (!pressed)
                        app.uiBackgroundOpacity = value / 100
                }
            }

            Text {
                text: app.hasHomeWallpaper
                      ? qsTr("100% 时壁纸全显示；调低后可透出桌面。0% 时仍保留约 18% 可见度。")
                      : qsTr("未设壁纸时为白底；调低后逐渐变透，0% 时仍保留约 18% 可见度。")
                font.pixelSize: 12
                color: Theme.textTertiary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                lineHeight: 1.4
            }
        }
    }

    SettingsCard {
        title: qsTr("卡片蒙层")
        subtitle: qsTr("侧栏、主区、底栏等卡片的白色半透明蒙层；内部按钮与文字保持实色")

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: qsTr("蒙层浓度")
                    font.pixelSize: 13
                    font.bold: true
                    color: Theme.textPrimary
                }
                Item { Layout.fillWidth: true }
                Text {
                    text: Math.round(app.uiCardShellOpacity * 100) + "%"
                    font.pixelSize: 13
                    color: Theme.accent
                    font.bold: true
                }
            }

            Slider {
                Layout.fillWidth: true
                from: 5
                to: 55
                stepSize: 1
                value: app.uiCardShellOpacity * 100
                onMoved: app.uiCardShellOpacity = value / 100
                onPressedChanged: {
                    if (!pressed)
                        app.uiCardShellOpacity = value / 100
                }
            }

            Text {
                text: qsTr("默认约 20% 白蒙层。数值越高卡片越实，越低越透；建议 15%～25%。")
                font.pixelSize: 12
                color: Theme.textTertiary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                lineHeight: 1.4
            }
        }
    }

    ColorDialog {
        id: textColorDialog
        title: qsTr("选择主文字颜色")
        selectedColor: app.uiTextColor
        onAccepted: app.uiTextColor = selectedColor
    }

    SettingsCard {
        title: qsTr("文字颜色")
        subtitle: qsTr("统一界面主文字与次要文字色调；启用后可自定义主色")

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: qsTr("自定义文字色")
                    font.pixelSize: 13
                    font.bold: true
                    color: Theme.textPrimary
                }
                Item { Layout.fillWidth: true }
                Switch {
                    checked: app.uiCustomTextColorEnabled
                    onToggled: app.uiCustomTextColorEnabled = checked
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    radius: Theme.radiusMd
                    color: app.uiCustomTextColorEnabled ? app.uiTextColor : Theme.textPrimary
                    border.color: Theme.borderStrong
                    border.width: 1
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Text {
                        text: app.uiCustomTextColorEnabled ? app.uiTextColor : Theme.textPrimary
                        font.pixelSize: 13
                        font.bold: true
                        color: Theme.textPrimary
                    }
                    Text {
                        text: qsTr("次要文字将自动按主色派生")
                        font.pixelSize: 12
                        color: Theme.textTertiary
                    }
                }

                ToolButton {
                    text: qsTr("选择颜色")
                    implicitHeight: 36
                    enabled: app.uiCustomTextColorEnabled
                    onClicked: textColorDialog.open()
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.accentSoft : Theme.bgCard
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

                ToolButton {
                    text: qsTr("恢复默认")
                    implicitHeight: 36
                    onClicked: {
                        app.uiTextColor = "#1B1B1F"
                        app.uiCustomTextColorEnabled = false
                    }
                    background: Rectangle {
                        radius: Theme.radiusMd
                        color: parent.hovered ? Theme.bgHover : Theme.bgCard
                        border.color: Theme.borderStrong
                    }
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 13
                        color: Theme.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Text {
                text: qsTr("未启用时使用统一默认色阶；标题、歌手、时间戳等将保持同一色系，避免过灰与过黑混杂。")
                font.pixelSize: 12
                color: Theme.textTertiary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                lineHeight: 1.4
            }
        }
    }
}
