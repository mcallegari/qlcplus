cmake_minimum_required(VERSION 3.16)
project(qlc VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 3DCore 3DInput 3DQuick 3DQuickExtras 3DRender Gui Multimedia MultimediaWidgets Network PrintSupport Qml Quick Svg Test Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Script)
find_package(PkgConfig REQUIRED)



add_subdirectory(hotplugmonitor)
add_subdirectory(engine)
add_subdirectory(resources)
add_subdirectory(plugins)
add_subdirectory(platforms)
if(qmlui)
    message("Building QLC+ 5 QML UI")
    add_subdirectory(qmlui)
else()
    message("Building QLC+ 4 QtWidget UI")
    add_subdirectory(ui)
    add_subdirectory(webaccess)
    add_subdirectory(main)
    add_subdirectory(fixtureeditor)
endif()
if(MACOS AND NOT qmlui)
    add_subdirectory(launcher)
endif()
