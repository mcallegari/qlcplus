cmake_minimum_required(VERSION 3.16)
project(uart VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui SerialPort Widgets)

add_library(uart)
target_sources(uart PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    uartplugin.cpp uartplugin.h
    uartwidget.cpp uartwidget.h
)
target_include_directories(uart PRIVATE
    ../interfaces
)

target_link_libraries(uart PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::SerialPort
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS uart
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
