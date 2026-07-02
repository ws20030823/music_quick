// SideNav.qml — 左侧导航（发现 / 我的音乐 / 创建的歌单）

import QtQuick

import QtQuick.Controls

import QtQuick.Layouts

import MusicQuick



Rectangle {

    id: root

    color: Theme.bgSidebar

    implicitWidth: Theme.sidebarWidth



    property int currentPage: 0
    property string activePlaylistId: ""
    signal navigate(int page)



    Rectangle {

        anchors.right: parent.right

        anchors.top: parent.top

        anchors.bottom: parent.bottom

        width: 1

        color: Theme.border

    }



    ScrollView {

        id: navScroll

        anchors.fill: parent

        anchors.margins: 12

        clip: true

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff



        Column {

            width: navScroll.availableWidth > 0 ? navScroll.availableWidth : root.width - 24

            spacing: 4



            SectionLabel { title: qsTr("发现") }

            NavButton { label: qsTr("推荐"); page: 0 }

            NavButton { label: qsTr("搜索"); page: 2 }



            SectionLabel { title: qsTr("我的音乐") }

            NavButton { label: qsTr("本地音乐"); page: 1; playlistId: "" }

            NavButton {

                label: qsTr("最近播放")

                page: -1

                enabled: false

            }

            NavButton {

                label: qsTr("我的收藏")

                page: -1

                enabled: false

            }



            Row {

                width: parent.width

                topPadding: 14

                bottomPadding: 4

                spacing: 4



                Text {

                    width: parent.width - addPlaylistBtn.width - parent.spacing

                    text: qsTr("创建的歌单")

                    font.pixelSize: 11

                    color: Theme.textTertiary

                    leftPadding: 4

                    elide: Text.ElideRight

                }



                ToolButton {

                    id: addPlaylistBtn

                    implicitWidth: 28

                    implicitHeight: 28

                    ToolTip.text: qsTr("新建歌单（即将推出）")

                    enabled: false

                    opacity: 0.45

                    background: Rectangle {

                        radius: Theme.radiusMd

                        color: parent.hovered ? Theme.bgHover : "transparent"

                    }

                    contentItem: Text {

                        text: "+"

                        font.pixelSize: 16

                        font.bold: true

                        color: Theme.textSecondary

                        horizontalAlignment: Text.AlignHCenter

                        verticalAlignment: Text.AlignVCenter

                    }

                }

            }



            NavButton {
                label: qsTr("我喜欢的音乐")
                page: 1
                playlistId: "liked"
                subtitle: app.trackCount > 0 ? qsTr("%1 首").arg(app.trackCount) : ""
            }

        }

    }



    component SectionLabel: Text {

        required property string title

        width: parent.width

        text: title

        font.pixelSize: 11

        color: Theme.textTertiary

        topPadding: 14

        bottomPadding: 4

        leftPadding: 4

    }



    component NavButton: Button {

        id: navBtn

        property string label

        property int page
        property string playlistId: ""
        property string subtitle: ""

        width: parent.width
        enabled: navBtn.page >= 0
        opacity: enabled ? 1 : 0.45
        flat: true
        checkable: enabled
        checked: enabled && root.currentPage === page && root.activePlaylistId === playlistId
        onClicked: {
            if (page < 0)
                return
            root.activePlaylistId = playlistId
            root.navigate(page)
        }



        background: Item {

            id: navBg

            implicitHeight: navBtn.subtitle !== "" ? 48 : 40

            implicitWidth: navBtn.width



            Rectangle {

                anchors.fill: parent

                radius: Theme.radiusMd

                color: navBg.parent.checked ? Theme.accentSoft

                     : (navBg.parent.hovered && navBg.parent.enabled ? Theme.bgHover : "transparent")

            }

            Rectangle {

                width: navBg.parent.checked ? 3 : 0

                anchors.left: parent.left

                anchors.top: parent.top

                anchors.bottom: parent.bottom

                radius: 2

                color: Theme.accent

            }

        }



        contentItem: Item {

            implicitHeight: navBtn.subtitle !== "" ? 48 : 40

            implicitWidth: navBtn.width



            Text {

                anchors.left: parent.left

                anchors.leftMargin: navBtn.checked ? 10 : 12

                anchors.verticalCenter: parent.verticalCenter

                anchors.verticalCenterOffset: navBtn.subtitle !== "" ? -8 : 0

                text: navBtn.label

                color: navBtn.checked ? Theme.accent : Theme.textSecondary

                font.pixelSize: 14

                font.weight: navBtn.checked ? Font.DemiBold : Font.Normal

                elide: Text.ElideRight

                width: parent.width - 16

            }

            Text {

                visible: navBtn.subtitle !== ""

                anchors.left: parent.left

                anchors.leftMargin: navBtn.checked ? 10 : 12

                anchors.top: parent.top

                anchors.topMargin: 26

                text: navBtn.subtitle

                color: Theme.textTertiary

                font.pixelSize: 11

            }

        }

    }

}

