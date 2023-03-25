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

qt_add_plugin(uart)
target_sources(uart PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    uartplugin.cpp uartplugin.h
    uartwidget.cpp uartwidget.h
)
target_include_directories(uart PRIVATE
    ../interfaces
)

target_link_libraries(uart PRIVATE
    Qt::Core
    Qt::Gui
    Qt::SerialPort
    Qt::Widgets
)

target_compile_options(uart
    -Werror
)

if(qmlui)
    target_compile_definitions(uart PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(uart
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(uart
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(uart PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS uart
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
