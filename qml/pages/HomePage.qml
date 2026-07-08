// HomePage.qml — 推荐首页（精选歌单网格）
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MusicQuick

Item {
    id: root

    readonly property int cardWidth: 180
    readonly property int gridGap: 25
    readonly property int columns: Math.max(2, Math.floor(
        (width - Theme.pagePadding * 2 + gridGap) / (cardWidth + gridGap)))

    ScrollView {
        anchors.fill: parent
        clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: root.width
            spacing: Theme.sectionGap

            Text {
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.topMargin: Theme.pagePadding
                text: qsTr("精选歌单")
                font.pixelSize: 22
                font.bold: true
                color: Theme.textPrimary
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: Theme.pagePadding
                Layout.rightMargin: Theme.pagePadding
                Layout.bottomMargin: Theme.pagePadding
                columns: root.columns
                columnSpacing: root.gridGap
                rowSpacing: root.gridGap + Theme.cardHoverLift

                Repeater {
                    model: app.featuredPlaylists

                    FeaturedPlaylistCard {
                        required property var modelData
                        Layout.preferredWidth: root.cardWidth
                        Layout.preferredHeight: root.cardWidth + 52
                        cardWidth: root.cardWidth
                        title: modelData.title
                        subtitle: modelData.subtitle
                        coverUrl: modelData.coverUrl
                        onClicked: app.openFeaturedPlaylist(modelData.id)
                    }
                }
            }
        }
    }
}
