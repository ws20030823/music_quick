# Music Quick

基于 Qt 6 Quick（QML）的桌面音乐播放器。界面用 QML 编写，播放与队列等逻辑在 C++ 中实现

## 功能

导入本地音频、歌曲列表、播放控制、进度与音量、四种播放模式、播放队列。

## 环境

- Qt 6.x（Quick、Quick Controls 2、Multimedia、Quick Effects）
- CMake 3.21+
- MinGW 64-bit 或 MSVC

## 构建

**Qt Creator：** 打开 `CMakeLists.txt`，选择 Qt 6 Kit，构建并运行 `MusicQuick`。

**命令行：**

```powershell
cmake -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64"
cmake --build build --config Release
.\build\MusicQuick.exe
```

## 测试

```powershell
cmake --build build --target MusicQuickTests
.\build\MusicQuickTests.exe
```

## 打包

Release 编译后执行：

```powershell
.\scripts\deploy.ps1
```

输出目录为 `dist\MusicQuick\`。需要安装包时，用 Inno Setup 编译 `installer\MusicQuick.iss`。
