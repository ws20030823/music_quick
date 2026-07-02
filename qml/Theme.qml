// Theme.qml — 浅色主题设计令牌（singleton）
// 全局 UI 颜色与尺寸常量，各 QML 通过 Theme.xxx 引用
pragma Singleton
import QtQuick

QtObject {
    // ── 背景色 ──

    // 主内容区背景（窗口底色、列表表头等）
    readonly property color bgBase: "#F3F3F7"

    // 左侧导航栏背景
    readonly property color bgSidebar: "#E4E8EE"

    // 卡片/顶栏/底栏/弹窗等白色面板
    readonly property color bgCard: "#FFFFFF"

    // 鼠标悬停时的半透明遮罩（约 10% 黑）
    readonly property color bgHover: "#0A000000"

    // ── 强调色 ──

    // 主强调色：选中态、进度条已播放部分、主按钮等
    readonly property color accent: "#0078D4"

    // 强调色 10% 透明：选中行背景、按钮按下态等
    readonly property color accentSoft: "#1A0078D4"

    // ── 文字色 ──

    // 主标题、歌曲名等重要文字
    readonly property color textPrimary: "#1B1B1F"

    // 副标题、歌手名、次要说明
    readonly property color textSecondary: "#6B7280"

    // 分区标签、时间戳、占位提示等最弱文字
    readonly property color textTertiary: "#9CA3AF"

    // ── 边框 ──

    // 默认分割线/边框（较淡）
    readonly property color border: "#0F000000"

    // 较强边框：播放按钮描边、滑块手柄描边、弹窗边框
    readonly property color borderStrong: "#D1D5DB"

    // ── 图标色 ──
    // SVG 源文件是黑色，AppIcon 当前直接显示原色；此属性供其他组件引用

    // 正常态图标
    readonly property color iconDefault: "#000000"

    // 禁用态图标（无歌曲、无队列时）
    readonly property color iconMuted: "#9CA3AF"

    // ── 滑块 ──

    // 进度条/音量条未填充轨道
    readonly property color sliderTrack: "#E5E7EB"

    // 滑块圆形手柄填充色（配合 borderStrong 描边）
    readonly property color sliderHandle: "#FFFFFF"

    // ── 布局尺寸 ──

    readonly property int sidebarWidth: 200
    readonly property int sidebarCollapsedWidth: 64
    readonly property int topBarHeight: 56
    readonly property int contentTabHeight: 44
    readonly property int playbackBarHeight: 88
    readonly property int pagePadding: 24
    readonly property int sectionGap: 20
    readonly property int radiusMd: 8
    readonly property int radiusLg: 12
    readonly property int coverSize: 48
    readonly property int listCoverSize: 40
    readonly property int logoSize: 24

    // ── 搜索页（ui-ux-pro-max：搜索 CTA + 卡片层次）──
    readonly property color searchHeroStart: "#E8F4FD"
    readonly property color searchHeroEnd: "#F3F3F7"
    readonly property int searchChipHeight: 32
    readonly property int searchHeroHeight: 120
}
