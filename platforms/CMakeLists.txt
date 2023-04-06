cmake_minimum_required(VERSION 3.16)
project(platforms VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)


if(UNIX AND NOT MACOS)
    add_subdirectory(linux)
endif()
if(WIN32)
    add_subdirectory(windows)
endif()
