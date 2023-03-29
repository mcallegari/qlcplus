include(../../variables.cmake)

cmake_minimum_required(VERSION 3.16)
project(dummy VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)

add_library(dummy)
target_sources(dummy PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    dummyconfiguration.cpp dummyconfiguration.h dummyconfiguration.ui
    dummyplugin.cpp dummyplugin.h
)
target_include_directories(dummy PRIVATE
    ../interfaces
)

target_link_libraries(dummy PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS dummy
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
