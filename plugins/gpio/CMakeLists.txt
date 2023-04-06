include(../../variables.cmake)

set(module_name "gpio")

set(TS_FILES
    GPIO_de_DE.ts
    GPIO_es_ES.ts
    GPIO_fi_FI.ts
    GPIO_fr_FR.ts
    GPIO_it_IT.ts
    GPIO_nl_NL.ts
    GPIO_cz_CZ.ts
    GPIO_pt_BR.ts
    GPIO_ca_ES.ts
    GPIO_ja_JP.ts
)

if(QT_VERSION_MAJOR GREATER 5)
    qt_add_translation(QM_FILES ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

cmake_minimum_required(VERSION 3.16)
project(${module_name} VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)

add_library(${module_name}
    ${QM_FILES}
)
target_sources(${module_name} PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    gpioconfiguration.cpp gpioconfiguration.h gpioconfiguration.ui
    gpioplugin.cpp gpioplugin.h
    gpioreaderthread.cpp gpioreaderthread.h
)
target_include_directories(${module_name} PRIVATE
    ../interfaces
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

install(TARGETS ${module_name}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${QM_FILES} DESTINATION ${INSTALLROOT}/${TRANSLATIONDIR})
