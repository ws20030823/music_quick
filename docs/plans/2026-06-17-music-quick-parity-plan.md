# Music Quick（QML）功能对齐计划

> 目标：在 `music_quick/` 用 **Qt Quick** 重建与 `music/`（Widgets）**当前进度一致**的播放器。  
> 策略：**复用 `core/` + `media/` C++**，UI 全部 QML，`AppController` 桥接。

---

## 功能对齐清单（对照 Widgets 版）

| 功能 | Widgets 现状 | QML 实现 |
|------|-------------|----------|
| 多选导入音频 | `QFileDialog` + `TrackMetadataReader` | `FileDialog` + 同 Reader |
| 元数据展示 | Delegate 列：序号/封面/标题/专辑/时长 | `TrackListModel` + `SongList` |
| 侧栏导航 | 首页 / 本地音乐 / 搜索 | `StackLayout` 三页 |
| 播放/暂停 | `AudioPlayer::togglePlayback` | `AppController::togglePlayback` |
| 上一首/下一首 | `PlaybackController` | 同 |
| 播放模式循环 | 顺序→随机→列表循环→单曲 | `cyclePlaybackMode` |
| 播完自动下一首 | `onEndOfMedia` | `mediaStatusChanged` 连接 |
| 进度条拖动 | `sliderMoved` | `seek(ms)` |
| 音量 | 0–100 滑块 | 同 |
| 播放队列面板 | `QueuePanelDialog` | `QueueDialog.qml` |
| 单元测试 | AudioPlayer / PlaybackMode / Controller | 复制核心测试，无 MainWindow |

**本阶段不做：** 歌单持久化、搜索功能、深色主题切换、QFluentKit。

---

## 目录结构

```
music_quick/
├── CMakeLists.txt
├── README.md
├── docs/plans/
├── src/
│   ├── main.cpp
│   ├── core/          # 从 music 复制
│   ├── media/         # 从 music 复制
│   └── app/
│       ├── AppController.h/.cpp
│       └── TrackListModel.h/.cpp
├── qml/
│   ├── Main.qml
│   ├── Theme.qml
│   ├── components/
│   └── pages/
└── tests/
    └── test_core.cpp
```

---

## Task 分步（执行顺序）

### Task 1: 脚手架 ✅
- 创建目录、复制 `core/` `media/`
- `CMakeLists.txt` + `main.cpp` + `Main.qml`

### Task 2: AppController + TrackListModel ✅
- C++ 桥接层：导入、播放、队列、模式切换
- 暴露 Q_PROPERTY 供 QML 绑定

### Task 3: QML 主壳 ✅
- 侧栏 + `StackLayout` 三页 + 底部播放栏
- `Theme.qml` 令牌（对齐 `UiTheme::Light`）

### Task 4: 本地音乐页 + 列表 ✅
- 表头列、封面占位、点击播放
- 正在播放高亮

### Task 5: 播放栏 + 队列弹窗 ✅
- 进度/音量/模式按钮/transport
- `QueueDialog`

### Task 6: 核心单元测试 ✅
- `tests/test_core.cpp`（无 MainWindow 依赖）

### Task 7: README 与验证说明 ✅

---

## 完成标准

1. `MusicQuick` 可启动，布局与 Widgets 版信息架构一致
2. 导入 → 播放 → 暂停 → 上一首/下一首 → 模式切换 → 队列面板 全流程可用
3. `MusicQuickTests` 核心测试通过
4. `core/` 无 QML/Widgets 依赖
