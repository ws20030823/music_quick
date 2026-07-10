// Theme.qml — 浅色主题设计令牌（singleton）
// 全局 UI 颜色与尺寸常量，各 QML 通过 Theme.xxx 引用
pragma Singleton
import QtQuick

QtObject {
    // ── 背景色 ──

    // 主内容区背景（窗口底色、卡片间隙）
    readonly property color bgBase: "#EBEEF3"

    // 左侧导航栏背景（卡片内与 bgCard 一致）
    readonly property color bgSidebar: "#FFFFFF"

    // 卡片/顶栏/底栏/弹窗等白色面板
    readonly property color bgCard: "#FFFFFF"

    // 顶栏背景（嵌入主内容卡片时为透明）
    readonly property color topBarBg: "transparent"

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
    readonly property int logoSize: 28

    // ── 卡片式外壳布局 ──
    readonly property int shellPadding: 12
    readonly property int cardGap: 10
    readonly property int cardShellRadius: 14
    readonly property color cardBorder: "#E2E8F0"
    readonly property color shellCardShadow: "#18000000"
    readonly property real cardShadowBlur: 0.45
    readonly property int cardShadowOffsetY: 3
    readonly property int settingsContentMaxWidth: 680

    // ── 搜索页（ui-ux-pro-max：搜索 CTA + 卡片层次）──
    readonly property color searchHeroStart: "#E8F4FD"
    readonly property color searchHeroEnd: bgBase
    readonly property int searchChipHeight: 32
    readonly property int searchHeroHeight: 120

    // ── 搜索结果表格（网易云布局）──
    readonly property int searchRowHeight: 56
    readonly property int searchTableHeaderHeight: 36
    readonly property int searchIndexWidth: 36
    readonly property int searchAlbumWidth: 160
    readonly property int searchLikeWidth: 40
    readonly property int searchDurationWidth: 48
    readonly property int searchFileSizeWidth: 56

    // ── 全屏播放页（浅色 + 黑胶左侧 / 白色歌词区）──
    readonly property color npBg: bgBase
    readonly property color npPanel: bgCard
    readonly property color npText: textPrimary
    readonly property color npTextMuted: textSecondary
    readonly property color npTextDim: textTertiary
    readonly property color npBorder: borderStrong
    readonly property color npLyricsBg: bgCard
    readonly property color npLyricsShadow: "#0D000000"
    readonly property color npVinylStage: "#E8EAEF"
    readonly property int npVinylSize: 320
    readonly property int npVinylRotationMs: 18000
    readonly property int npTonearmAnimMs: 550
    readonly property real npTonearmPlayingDeg: 22
    readonly property real npTonearmRestDeg: -28

    // ── 歌词面板（网易云风格）──
    readonly property string npLyricFontFamily: "Microsoft YaHei UI"
    readonly property color npLyricActive: "#222222"
    readonly property color npLyricInactive: "#B2B2B2"
    readonly property int npLyricActiveSize: 26
    readonly property int npLyricInactiveSize: 17
    readonly property int npLyricLineHeight: 52
    readonly property int npLyricPaddingH: 32
    readonly property int npLyricFadeHeight: 72
    readonly property int npLyricScrollMs: 600
    readonly property int npLyricAnimMs: 280

    // ── 首页歌单卡片 ──
    readonly property int cardCoverRadius: 12
    readonly property color cardShadow: "#14000000"
    readonly property int cardHoverLift: 5
    readonly property int cardCoverHoverScaleMs: 300

    // ── 交互卡片对话框（Uiverse 风格）──
    readonly property int dialogRadius: 20
    readonly property int dialogPadding: 30
    readonly property int dialogGap: 20
    readonly property int dialogContentGap: 5
    readonly property int dialogMinWidth: 300
    readonly property int dialogButtonHeight: 35
    readonly property int dialogButtonRadius: 10
    readonly property int dialogButtonGap: 10
    readonly property int dialogShadowOffset: 20
    readonly property color dialogBg: "#FFFFFF"
    readonly property color dialogHeading: "#1B1B1B"
    readonly property color dialogDescription: "#666666"
    readonly property color dialogPrimary: "#FF726D"
    readonly property color dialogPrimaryHover: "#FF4942"
    readonly property color dialogSecondary: "#DDDDDD"
    readonly property color dialogSecondaryHover: "#C5C5C5"
    readonly property color dialogExit: "#AFAFAF"
    readonly property color dialogExitHover: "#1B1B1B"
    readonly property color dialogShadow: "#12000000"

    // ── 皮肤透明度（0% 滑块仍保留最低可见度）──
    readonly property real skinOpacityMin: 0.28

    function mapSkinOpacity(raw) {
        var clamped = Math.max(0, Math.min(1, raw))
        return skinOpacityMin + clamped * (1.0 - skinOpacityMin)
    }

    // 无障碍：后续可绑定 SystemSettings 或平台 reduced-motion
    readonly property bool reduceMotion: false
}
