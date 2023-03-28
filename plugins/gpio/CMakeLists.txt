cmake_minimum_required(VERSION 3.16)
project(gpio VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)

add_library(gpio)
target_sources(gpio PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    gpioconfiguration.cpp gpioconfiguration.h gpioconfiguration.ui
    gpioplugin.cpp gpioplugin.h
    gpioreaderthread.cpp gpioreaderthread.h
)
target_include_directories(gpio PRIVATE
    ../interfaces
)

target_link_libraries(gpio PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS gpio
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
