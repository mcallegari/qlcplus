cmake_minimum_required(VERSION 3.16)
project(plugins VERSION 1.0 LANGUAGES C CXX)
find_package(PkgConfig REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)

pkg_check_modules(MAD IMPORTED_TARGET mad)

if((((NOT ANDROID AND NOT IOS) AND (${MAD_FOUND}))))
    add_subdirectory(mad)
endif()

pkg_check_modules(SNDFILE IMPORTED_TARGET sndfile)
if((((NOT ANDROID AND NOT IOS) AND (${SNDFILE_FOUND}))))
    add_subdirectory(sndfile)
endif()
