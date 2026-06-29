cmake_minimum_required(VERSION 3.21)

project(MusicQuick LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 QuickEffects Multimedia Test)

qt_standard_project_setup()

# singleton 必须在 CMake 中显式注册，否则 Theme.xxx / Icons.xxx 运行时会失效
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/Icons.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

enable_testing()

qt_add_executable(MusicQuick
    src/main.cpp
    src/core/AudioPlayer.cpp
    src/core/AudioPlayer.h
    src/core/PlaybackController.cpp
    src/core/PlaybackController.h
    src/media/TrackMetadataReader.cpp
    src/media/TrackMetadataReader.h
    src/media/TrackMetadata.h
    src/app/AppController.cpp
    src/app/AppController.h
    src/app/TrackListModel.cpp
    src/app/TrackListModel.h
)

qt_add_resources(MusicQuick app_icons resources/app_icons.qrc)

qt_add_qml_module(MusicQuick
    URI MusicQuick
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/Icons.qml
        qml/components/AppIcon.qml
        qml/components/SideNav.qml
        qml/components/PlaybackBar.qml
        qml/components/SongList.qml
        qml/components/QueueDialog.qml
        qml/pages/HomePage.qml
        qml/pages/LocalMusicPage.qml
        qml/pages/SearchPage.qml
)

target_include_directories(MusicQuick PRIVATE "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(MusicQuick PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickEffects
    Qt6::Multimedia
)

set_target_properties(MusicQuick PROPERTIES WIN32_EXECUTABLE TRUE)

qt_add_executable(MusicQuickTests
    tests/test_core.cpp
    src/core/AudioPlayer.cpp
    src/core/AudioPlayer.h
    src/core/PlaybackController.cpp
    src/core/PlaybackController.h
    src/media/TrackMetadataReader.cpp
    src/media/TrackMetadataReader.h
    src/media/TrackMetadata.h
)

target_include_directories(MusicQuickTests PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(MusicQuickTests PRIVATE Qt6::Test Qt6::Multimedia)

add_test(NAME musicquick_core COMMAND MusicQuickTests)
