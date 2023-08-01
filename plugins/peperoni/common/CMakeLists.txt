project(common)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

add_executable(common WIN32 MACOSX_BUNDLE
    inputdevice.cpp inputdevice.h
    iodevice.cpp iodevice.h
    ioenumerator.cpp ioenumerator.h
    main.cpp
    outputdevice.cpp outputdevice.h
)

target_link_libraries(common PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

if(WIN32)
    target_sources(common PRIVATE
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

if(APPLE)
    set_target_properties(common PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(UNIX)
    target_sources(common PRIVATE
        unixioenumerator.cpp unixioenumerator.h
        unixpeperonidevice.cpp unixpeperonidevice.h
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
