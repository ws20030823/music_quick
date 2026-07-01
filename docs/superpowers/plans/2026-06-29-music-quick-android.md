# Music Quick Android 手机版 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在现有 `music_quick` 基础上增加 Android 目标，产出可安装运行的 APK，保留当前 C++ 播放核心，手机端提供可用的导入、列表、播放控制体验。

**Architecture:** 复用 `src/core/`、`src/media/`、`src/app/`（`AppController` + `TrackListModel`）不变；QML 按平台分流——桌面继续用 `Main.qml`，Android 使用新建的 `MainMobile.qml` + 底部 Tab 导航；文件访问通过新增 `PlatformFileAccess` 统一本地路径与 Android `content://` URI；CMake 增加 Android 工具链与 `android/` 清单资源，用 `androiddeployqt` 打 APK。

**Tech Stack:** Qt 6.8+（Quick、Quick Controls 2、Multimedia、Quick Effects）、CMake 3.21+、Android SDK/NDK（经 Qt Maintenance Tool 安装）、JDK 17、Gradle（Qt 自带 wrapper）

## Global Constraints

- 目标平台：Android 8.0+（API 26+），优先 arm64-v8a
- 首版只做 **Android**；iOS 不在本计划范围
- 复用现有 C++ 业务层，不在本阶段重写播放引擎
- UI 首版不做像素级复刻桌面侧栏，采用手机常见 **底部 Tab + 全屏列表 + 迷你播放条**
- 首版不做：后台播放通知、锁屏控制、歌单持久化、在线搜索、应用商店上架签名流水线
- 图标染色继续用 `MultiEffect`；Android Kit 必须安装 **Qt Quick Effects** 模块（与桌面相同）
- 所有用户可见文案使用简体中文
- 每个 Task 完成后必须能在真机或模拟器上验证，再进入下一 Task

---

## 能不能现在做？结论

| 维度 | 评估 |
|------|------|
| 技术可行性 | **可以**。项目已是 Qt Quick + C++ 分层，比 Widgets 版更适合移植 Android |
| 现成可用 | **不能**。当前 CMake 仅 Windows 桌面；`Main.qml` 为 1100×720 桌面布局；`importFiles` 假设本地文件路径 |
| 工作量 | 中等偏大：环境 0.5–1 天 + 首个 APK 1 天 + 文件/布局/真机调试 2–4 天 |
| 硬件要求 | 需安装 Android SDK/NDK、JDK；真机 USB 调试强烈建议 |

**建议节奏：** 先完成 **Task 1–3** 打出「能启动的空壳 APK」，再补文件导入与播放，避免一次改太多。

---

## 文件结构（计划新增/修改）

| 路径 | 职责 |
|------|------|
| `CMakeLists.txt` | 增加 Android 分支、`android/` 资源、移除仅 Windows 的全局属性 |
| `android/AndroidManifest.xml` | 权限、`READ_MEDIA_AUDIO`、Activity 配置 |
| `android/res/values/libs.xml` | Qt 自动生成/维护的 native libs 声明 |
| `src/platform/PlatformFileAccess.h/.cpp` | 统一 `QUrl` → 可读路径 / `QIODevice` |
| `src/app/AppController.cpp` | `importFiles` 改走 `PlatformFileAccess` |
| `src/media/TrackMetadataReader.cpp` | 支持从 `QIODevice`/临时文件读标签 |
| `qml/MainMobile.qml` | Android 主窗口（StackView + 底部 Tab） |
| `qml/mobile/MobilePlaybackBar.qml` | 紧凑播放条 |
| `qml/mobile/MobileBottomNav.qml` | 首页 / 本地 / 搜索 Tab |
| `src/main.cpp` | 按平台加载 `Main` 或 `MainMobile` |
| `scripts/deploy-android.ps1` | 一键 cmake + androiddeployqt |
| `README.md` | 补充 Android 构建说明 |
| `tests/test_platform_file_access.cpp` | 桌面侧 URI/路径转换单测 |

桌面现有文件 **保留**：`qml/Main.qml`、`SideNav.qml` 等不删除，仅 Android 不加载。

---

### Task 1: Android 构建环境就绪

**Files:**
- Modify: `README.md`
- Create: `docs/superpowers/plans/android-prerequisites-checklist.md`（可选，本 Task 步骤已足够）

**Interfaces:**
- Consumes: 无
- Produces: 开发者机器上可用的 Qt Android Kit（例如 `Qt 6.8.3 Android arm64-v8a`）

- [ ] **Step 1: 安装 Android 组件**

打开 Qt Maintenance Tool，在已安装的 Qt 6.8.x 下勾选：

- Android arm64-v8a（或 armeabi-v7a + arm64-v8a）
- Android SDK / NDK / OpenSSL（如安装器提供）
- **Qt Quick Effects**（与桌面相同，避免 CMake 缺模块）

- [ ] **Step 2: 安装 JDK 17**

安装 Eclipse Temurin 17 或 Oracle JDK 17，设置环境变量：

```powershell
$env:JAVA_HOME = "C:\Program Files\Eclipse Adoptium\jdk-17.0.x"
$env:PATH = "$env:JAVA_HOME\bin;$env:PATH"
java -version
```

Expected: 输出 `openjdk version "17.x"`

- [ ] **Step 3: 在 Qt Creator 验证 Kit**

Qt Creator → 编辑 → Kits：应出现 **Android Qt 6.8.x Clang arm64-v8a**。Device 里可创建 Android 模拟器或连接真机（USB 调试）。

- [ ] **Step 4: Commit**

```bash
git add README.md
git commit -m "docs: add Android build prerequisites"
```

---

### Task 2: CMake Android 脚手架 + 最小 APK

**Files:**
- Modify: `CMakeLists.txt`
- Create: `android/AndroidManifest.xml`
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: Qt6 Android 工具链
- Produces: 平台无关的 `MusicQuick` 目标；Android 上加载 `MainMobile.qml`（Task 4 前可暂用简化占位）

- [ ] **Step 1: 调整 CMakeLists.txt 支持 Android**

在 `find_package(Qt6 ...)` 之后、`qt_add_executable` 之前增加：

```cmake
if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
endif()
```

将：

```cmake
set_target_properties(MusicQuick PROPERTIES WIN32_EXECUTABLE TRUE)
```

改为：

```cmake
if(WIN32)
    set_target_properties(MusicQuick PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
```

在 `qt_add_qml_module` 的 `QML_FILES` 列表中 **暂不** 加 `MainMobile.qml`（Task 4 再加）；本 Task 先验证 Android 能编过。

- [ ] **Step 2: 创建 AndroidManifest.xml**

`android/AndroidManifest.xml`:

```xml
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.musicquick.app">
    <uses-permission android:name="android.permission.READ_MEDIA_AUDIO" />
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" android:maxSdkVersion="32" />
    <application
        android:label="Music Quick"
        android:extractNativeLibs="true">
        <activity
            android:name="org.qtproject.qt.android.bindings.QtActivity"
            android:configChanges="orientation|uiMode|screenLayout|screenSize|smallestScreenSize|layoutDirection|locale|fontScale|keyboard|keyboardHidden|navigation|mcc|mnc|density"
            android:launchMode="singleTop"
            android:exported="true"
            android:screenOrientation="unspecified">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
```

- [ ] **Step 3: 用 Qt Creator 构建 APK**

1. Kit 选 **Android Qt 6.8.x arm64-v8a**
2. 构建 `MusicQuick`
3. 菜单 **Build → Build Android APK**（或 Deploy 到已连接设备）

Expected: 生成 `build-.../android-build/build/outputs/apk/debug/android-build-debug.apk`

- [ ] **Step 4: 真机/模拟器安装验证**

```powershell
adb install -r path\to\android-build-debug.apk
```

Expected: 应用图标出现，启动后显示当前 `Main.qml`（可能布局挤压，本 Task 只要求 **不闪退**）

- [ ] **Step 5: Commit**

```bash
git add CMakeLists.txt android/AndroidManifest.xml src/main.cpp
git commit -m "feat: add Android CMake scaffold and manifest"
```

---

### Task 3: 平台文件访问层（TDD）

**Files:**
- Create: `src/platform/PlatformFileAccess.h`
- Create: `src/platform/PlatformFileAccess.cpp`
- Create: `tests/test_platform_file_access.cpp`
- Modify: `CMakeLists.txt`（加入源文件与测试目标）

**Interfaces:**
- Consumes: `QUrl`, `QFile`, `QTemporaryFile`
- Produces:
  - `QString PlatformFileAccess::readableLocalPath(const QUrl& url)` — 桌面返回 `toLocalFile()`；Android `content://` 复制到临时文件并返回路径
  - `bool PlatformFileAccess::isSupportedImportUrl(const QUrl& url)` — 过滤无效 scheme

- [ ] **Step 1: Write the failing test**

`tests/test_platform_file_access.cpp`:

```cpp
#include "platform/PlatformFileAccess.h"
#include <QtTest>
#include <QUrl>

class PlatformFileAccessTest : public QObject {
    Q_OBJECT
private slots:
    void localFileUrl_returnsSamePath() {
        const QUrl url = QUrl::fromLocalFile(QStringLiteral("C:/music/test.mp3"));
        QCOMPARE(PlatformFileAccess::readableLocalPath(url), QStringLiteral("C:/music/test.mp3"));
    }
    void emptyUrl_returnsEmpty() {
        QCOMPARE(PlatformFileAccess::readableLocalPath(QUrl()), QString());
    }
};

QTEST_MAIN(PlatformFileAccessTest)
#include "test_platform_file_access.moc"
```

- [ ] **Step 2: Run test to verify it fails**

```powershell
cmake --build build --target MusicQuickTests
.\build\MusicQuickTests.exe
```

Expected: FAIL — `PlatformFileAccess` 未定义

- [ ] **Step 3: Minimal implementation**

`src/platform/PlatformFileAccess.h`:

```cpp
#pragma once
#include <QUrl>
#include <QString>

namespace PlatformFileAccess {
QString readableLocalPath(const QUrl& url);
bool isSupportedImportUrl(const QUrl& url);
}
```

`src/platform/PlatformFileAccess.cpp`:

```cpp
#include "platform/PlatformFileAccess.h"
#include <QFile>
#include <QTemporaryFile>
#include <QFileInfo>

QString PlatformFileAccess::readableLocalPath(const QUrl& url)
{
    if (!url.isValid()) {
        return {};
    }
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
#if defined(Q_OS_ANDROID)
    if (url.scheme() == QLatin1String("content")) {
        QFile src(url.toString(QUrl::FullyEncoded));
        if (!src.open(QIODevice::ReadOnly)) {
            return {};
        }
        QTemporaryFile tmp;
        tmp.setAutoRemove(false);
        if (!tmp.open()) {
            return {};
        }
        tmp.write(src.readAll());
        tmp.close();
        return tmp.fileName();
    }
#endif
    return url.toString();
}

bool PlatformFileAccess::isSupportedImportUrl(const QUrl& url)
{
    if (!url.isValid()) {
        return false;
    }
    if (url.isLocalFile()) {
        return true;
    }
#if defined(Q_OS_ANDROID)
    return url.scheme() == QLatin1String("content");
#else
    return false;
#endif
}
```

CMake：把上述 `.cpp` 加入 `MusicQuick` 与 `MusicQuickTests`（或单独测试 target）。

- [ ] **Step 4: Run test to verify it passes**

Expected: `localFileUrl_returnsSamePath` PASS

- [ ] **Step 5: Wire AppController**

`AppController.cpp` `importFiles` 循环内改为：

```cpp
const QUrl& url = urls[i];
if (!PlatformFileAccess::isSupportedImportUrl(url)) {
    continue;
}
const QString path = PlatformFileAccess::readableLocalPath(url);
```

- [ ] **Step 6: Commit**

```bash
git add src/platform/ tests/test_platform_file_access.cpp src/app/AppController.cpp CMakeLists.txt
git commit -m "feat: add PlatformFileAccess for Android content URIs"
```

---

### Task 4: 手机版 QML 主壳

**Files:**
- Create: `qml/MainMobile.qml`
- Create: `qml/mobile/MobileBottomNav.qml`
- Create: `qml/mobile/MobilePlaybackBar.qml`
- Modify: `CMakeLists.txt`（QML_FILES 注册）
- Modify: `src/main.cpp`

**Interfaces:**
- Consumes: 全局 `app`（`AppController`）、`Theme` singleton
- Produces: Android 启动加载 `MainMobile`；桌面仍加载 `Main`

- [ ] **Step 1: main.cpp 按平台分流**

```cpp
#if defined(Q_OS_ANDROID)
    engine.loadFromModule(QStringLiteral("MusicQuick"), QStringLiteral("MainMobile"));
#else
    engine.loadFromModule(QStringLiteral("MusicQuick"), QStringLiteral("Main"));
#endif
```

- [ ] **Step 2: 创建 MainMobile.qml**

要点：

- `ApplicationWindow` 全屏，`flags: Qt.Window`
- 去掉 `minimumWidth/Height` 桌面约束
- 结构：`ColumnLayout` → `StackLayout`（三页，复用现有 `HomePage`/`LocalMusicPage`/`SearchPage`）→ `MobilePlaybackBar` → `MobileBottomNav`
- 侧栏 `SideNav` **不使用**
- `FileDialog` 保留，`onAccepted: app.importFiles(selectedFiles)`

`MainMobile.qml` 骨架：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import MusicQuick

ApplicationWindow {
    id: root
    visible: true
    width: 360
    height: 640
    title: qsTr("Music Quick")
    color: Theme.bgBase

    FileDialog {
        id: importDialog
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("音频文件 (*.mp3 *.wav *.flac *.aac)")]
        onAccepted: app.importFiles(selectedFiles)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: app.currentPage
            HomePage { }
            LocalMusicPage {
                trackModel: app.trackModel
                onImportClicked: importDialog.open()
            }
            SearchPage { }
        }

        MobilePlaybackBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 72
        }

        MobileBottomNav {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            currentPage: app.currentPage
            onNavigate: function(page) { app.currentPage = page }
            onImportClicked: importDialog.open()
        }
    }
}
```

- [ ] **Step 3: MobilePlaybackBar / MobileBottomNav**

从 `PlaybackBar.qml`、`SideNav.qml` **复制并简化**：

- 播放按钮、上一首/下一首、标题一行、进度条保留
- 去掉桌面专用宽度；触控目标最小 44×44 dp
- 底部 Tab：首页 / 本地 / 搜索 + 导入按钮

- [ ] **Step 4: 调整 Theme.qml 移动端尺寸（可选属性）**

```qml
readonly property bool isMobile: Qt.platform.os === "android" || Qt.platform.os === "ios"
readonly property int playbackBarHeight: isMobile ? 72 : 88
```

- [ ] **Step 5: Android 构建并在真机验证布局**

Expected: 三页可切换，底部播放条可见，无侧栏，竖屏不溢出

- [ ] **Step 6: Commit**

```bash
git add qml/MainMobile.qml qml/mobile/ src/main.cpp qml/Theme.qml CMakeLists.txt
git commit -m "feat: add mobile QML shell for Android"
```

---

### Task 5: Android 导入与播放真机验证

**Files:**
- Modify: `qml/pages/LocalMusicPage.qml`（移动端列表行高、触控区域）
- Modify: `src/core/AudioPlayer.cpp`（必要时记录 `load` 失败日志）
- Modify: `android/AndroidManifest.xml`（若 Android 13+ 需运行时权限）

**Interfaces:**
- Consumes: `PlatformFileAccess`, `AppController::importFiles`, `FileDialog`
- Produces: 真机上可导入 mp3 并听到声音

- [ ] **Step 1: 本地音乐页移动端适配**

`SongList.qml` 或 `LocalMusicPage.qml`：

- 行高 ≥ 56
- 整行 `MouseArea` 触发 `app.playRow(index)`

- [ ] **Step 2: 真机导入测试**

1. 安装 APK，打开「本地音乐」
2. 点导入，从 Downloads 选 mp3
3. 观察 `app.statusText` 是否「已导入 N 首歌曲」

Expected: 列表出现曲目；点一行开始播放

- [ ] **Step 3: 播放控制测试**

验证：播放/暂停、拖动进度、下一首、模式切换、队列弹窗（可改为全屏 BottomSheet 样式，首版可用现有 `QueueDialog`）

- [ ] **Step 4: 若无声或加载失败**

检查 logcat：

```powershell
adb logcat | Select-String -Pattern "QMediaPlayer|MusicQuick|Android"
```

常见修复：`AudioPlayer::load` 确认传入路径在 Android 上为可读临时文件；`content://` 未复制成功则回到 Task 3。

- [ ] **Step 5: Commit**

```bash
git add qml/ src/core/AudioPlayer.cpp android/AndroidManifest.xml
git commit -m "fix: Android import and playback on device"
```

---

### Task 6: APK 打包脚本与文档

**Files:**
- Create: `scripts/deploy-android.ps1`
- Modify: `README.md`

**Interfaces:**
- Consumes: Release/Debug Android 构建产物
- Produces: 可分发 `MusicQuick-debug.apk` 或 signed release apk 说明

- [ ] **Step 1: deploy-android.ps1**

```powershell
param(
    [string]$QtAndroid = "C:/Qt/6.8.3/android_arm64_v8a",
    [string]$BuildDir = "build-android"
)

$ErrorActionPreference = "Stop"
$Ndk = "$QtAndroid/lib/cmake/Qt6/qt.toolchain.cmake"

cmake -B $BuildDir -DCMAKE_TOOLCHAIN_FILE="$Ndk" -DANDROID_ABI=arm64-v8a -DCMAKE_PREFIX_PATH="$QtAndroid"
cmake --build $BuildDir
cmake --build $BuildDir --target apk
Write-Host "APK in $BuildDir/android-build/build/outputs/apk/"
```

（实际 toolchain 路径以本机 Qt 安装为准，脚本内注释说明如何查找。）

- [ ] **Step 2: 更新 README Android 小节**

包含：所需 Qt 模块、JDK、构建步骤、`adb install`、常见错误（缺 QuickEffects、缺 NDK）

- [ ] **Step 3: Commit**

```bash
git add scripts/deploy-android.ps1 README.md
git commit -m "docs: add Android APK build script and README"
```

---

## 风险与首版已知限制

| 风险 | 缓解 |
|------|------|
| `content://` 大文件复制慢 | 首版接受；后续用 `QMediaPlayer` 直接 setSource(url) 或持久化缓存 |
| `QuickEffects` 未安装 | Maintenance Tool 勾选；或改 `AppIcon.qml` 去掉 `MultiEffect` |
| 桌面 `TrackListUtils` 等 Widgets 残留 | 不要编入 Android 目标；清理无用文件单独 PR |
| 切后台播放暂停 | 首版不保证；后续加 `QtMultimedia` 后台服务 / 通知 |
| Release 签名 | 首版 debug APK 自用；上架需 keystore + `androiddeployqt --release` |

---

## Self-Review

| 检查项 | 结果 |
|--------|------|
| Spec：手机版 + APK | Task 2、6 覆盖 |
| Spec：复用 C++ 核心 | Architecture 与 Task 3、5 仅改 `AppController` 路径层 |
| Spec：可用导入/播放 | Task 3、5 |
| 占位符扫描 | 无 TBD；各步含具体路径与代码 |
| 类型一致性 | `PlatformFileAccess::readableLocalPath` 在 Task 3 定义、Task 5 使用，一致 |

---

## 你应该先手动做什么

1. 确认是否已安装 **Qt for Android** 与 **JDK 17**（Task 1）
2. 准备一台 Android 真机或模拟器
3. 从 **Task 2** 开始，先打出能启动的 APK，再做 UI 与导入

## 暂时不要做什么

- 不要试图一份 QML 同时完美适配桌面与手机（维护成本高）
- 不要在本阶段做 iOS、应用商店上架、后台播放
- 不要为 Android 引入 Flutter/React Native 等第二套 UI 栈

## 下一步建议

完成 Task 1 环境检查后即可开 Task 2；若 CMake 报错缺 `QuickEffects`，先在 Maintenance Tool 安装该模块（与桌面 clone 问题相同）。
