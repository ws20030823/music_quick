// SettingsThemePanel.qml — 换肤
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import MusicQuick

SettingsPanelLayout {
    title: qsTr("换肤")
    subtitle: qsTr("自定义壁纸与界面不透明度")

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
        title: qsTr("皮肤不透明度")
        subtitle: qsTr("数值越低，背景与卡片越透明；无壁纸时默认为白底")

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: qsTr("不透明度")
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
                      ? qsTr("100% 时壁纸与界面较实；调低后可透出桌面。滑到 0% 时仍保留约 28% 可见度。")
                      : qsTr("未设壁纸时为白底；调低不透明度后逐渐变透，0% 时仍保留约 28% 可见度。")
                font.pixelSize: 12
                color: Theme.textTertiary
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                lineHeight: 1.4
            }
        }
    }
}
