cmake_minimum_required(VERSION 3.16)
project(plugins VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)


add_subdirectory(artnet)
add_subdirectory(E1.31)
add_subdirectory(loopback)
add_subdirectory(osc)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(dmxusb)
    add_subdirectory(peperoni)
    add_subdirectory(udmx)
    add_subdirectory(midi)
    add_subdirectory(velleman)
    add_subdirectory(enttecwing)
    add_subdirectory(hid)
    add_subdirectory(os2l)
endif()

pkg_check_modules(LIBOLA IMPORTED_TARGET libola)
pkg_check_modules(LIBOLASERVER IMPORTED_TARGET libolaserver)

if((((NOT ANDROID AND NOT IOS) AND (UNIX) AND (${LIBOLA_FOUND}) AND (${LIBOLASERVER_FOUND}))))
    add_subdirectory(ola)
endif()
if(UNIX AND NOT ANDROID AND NOT IOS AND NOT MACOS)
    add_subdirectory(dmx4linux)
    add_subdirectory(spi)
endif()
