cmake_minimum_required(VERSION 3.21)

project(MusicQuick LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 QuickEffects Multimedia Network)
if(NOT ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

qt_standard_project_setup()

if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
endif()

# singleton 必须在 CMake 中显式注册，否则 Theme.xxx / Icons.xxx 运行时会失效
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/Icons.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

if(NOT ANDROID)
    enable_testing()
endif()

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
    src/app/SearchResultModel.cpp
    src/app/SearchResultModel.h
    src/network/MyFreeMp3Parser.cpp
    src/network/MyFreeMp3Parser.h
    src/network/MyFreeMp3Client.cpp
    src/network/MyFreeMp3Client.h
    src/network/OnlineStreamLoader.cpp
    src/network/OnlineStreamLoader.h
    src/network/OnlineTrack.h
)

set(MUSICQUICK_ICON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons")
set(MUSICQUICK_ICON_NAMES
    play.svg pause.svg prev.svg next.svg
    shuffle.svg repeat-all.svg repeat-one.svg
    playlist.svg volume.svg
    minimize.svg maximize.svg restore.svg close.svg
)
set(MUSICQUICK_ICON_FILES "")
foreach(_icon IN LISTS MUSICQUICK_ICON_NAMES)
    set(_icon_path "${MUSICQUICK_ICON_DIR}/${_icon}")
    set_source_files_properties("${_icon_path}" PROPERTIES QT_RESOURCE_ALIAS "${_icon}")
    list(APPEND MUSICQUICK_ICON_FILES "${_icon_path}")
endforeach()
set(MUSICQUICK_LOGO_FILE "${MUSICQUICK_ICON_DIR}/GankDream-03-origami-dove.svg")
set_source_files_properties("${MUSICQUICK_LOGO_FILE}" PROPERTIES QT_RESOURCE_ALIAS "logo.svg")
list(APPEND MUSICQUICK_ICON_FILES "${MUSICQUICK_LOGO_FILE}")
qt_add_resources(MusicQuick "app_icons"
    PREFIX "/icons"
    FILES ${MUSICQUICK_ICON_FILES}
)

qt_add_qml_module(MusicQuick
    URI MusicQuick
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/Icons.qml
        qml/components/AppIcon.qml
        qml/components/TopBar.qml
        qml/components/WindowControls.qml
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
    Qt6::Network
)

if(WIN32 AND NOT ANDROID)
    set_target_properties(MusicQuick PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(NOT ANDROID)
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
endif()
