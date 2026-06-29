pragma Singleton
import QtQuick

// =============================================================================
// Icons.qml — SVG 图标注册表（singleton）
// =============================================================================
// 图标文件位于 resources/icons/，经 qrc:/icons/<name>.svg 加载。
// 新增图标：放入 resources/icons/ 并在 CMake RESOURCES 中注册，再于此添加常量。
// =============================================================================
QtObject {
    // 播放控制
    readonly property string play: "play"
    readonly property string pause: "pause"
    readonly property string prev: "prev"
    readonly property string next: "next"
    readonly property string volume: "volume"
    readonly property string playlist: "playlist"

    // 播放模式（顺序模式暂用 playlist 图标，后续可换 list-ordered.svg）
    readonly property string shuffle: "shuffle"
    readonly property string repeatAll: "repeat-all"
    readonly property string repeatOne: "repeat-one"

    // 返回 qrc 路径
    function url(name) {
        return "qrc:/icons/" + name + ".svg"
    }

    // 根据 app.playbackMode 枚举返回图标名
    function modeIconName(mode) {
        switch (mode) {
        case 0: return playlist
        case 1: return shuffle
        case 2: return repeatAll
        case 3: return repeatOne
        default: return playlist
        }
    }
}
