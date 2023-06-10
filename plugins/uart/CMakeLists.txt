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

set(module_name "uart")

set(TS_FILES
    UART_de_DE.ts
    UART_es_ES.ts
    UART_fi_FI.ts
    UART_fr_FR.ts
    UART_it_IT.ts
    UART_nl_NL.ts
    UART_cz_CZ.ts
    UART_pt_BR.ts
    UART_ca_ES.ts
    UART_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_library(${module_name} SHARED
    ${QM_FILES}
)

target_sources(${module_name} PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    uartplugin.cpp uartplugin.h
    uartwidget.cpp uartwidget.h
)
target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::SerialPort
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    DESTINATION ${INSTALLROOT}/${PLUGINDIR}
)

install(FILES ${QM_FILES} DESTINATION ${INSTALLROOT}/${TRANSLATIONDIR})
