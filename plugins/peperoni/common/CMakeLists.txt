cmake_minimum_required(VERSION 3.16)
project(common VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

qt_add_executable(common WIN32 MACOSX_BUNDLE
    inputdevice.cpp inputdevice.h
    iodevice.cpp iodevice.h
    ioenumerator.cpp ioenumerator.h
    main.cpp
    outputdevice.cpp outputdevice.h
)
target_link_libraries(common PRIVATE
    Qt::Core
    Qt::Gui
)

if(WIN32)
    target_sources(common PUBLIC
        ../win32/peperoni/usbdmx-dynamic.cpp ../win32/peperoni/usbdmx-dynamic.h
        win32ioenumerator.cpp win32ioenumerator.h
        win32peperonidevice.cpp win32peperonidevice.h
    )

    set_target_properties(common PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    target_include_directories(common PRIVATE
        ../win32/peperoni
    )
endif()

if(MACOS)
    set_target_properties(common PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(UNIX)
    target_sources(common PUBLIC
        unixioenumerator.cpp unixioenumerator.h
        unixpeperonidevice.cpp unixpeperonidevice.h
    )
endif()

install(TARGETS common
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
