qt_add_plugin(hidplugin)
target_sources(hidplugin PRIVATE
    ../interfaces/qlcioplugin.cpp ../interfaces/qlcioplugin.h
    configurehid.cpp configurehid.h configurehid.ui
    hiddevice.cpp hiddevice.h
    hiddmxdevice.cpp hiddmxdevice.h
    hidjsdevice.cpp hidjsdevice.h
    hidplugin.cpp hidplugin.h
)
target_include_directories(hidplugin PRIVATE
    ../interfaces
)

target_link_libraries(hidplugin PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_compile_options(hidplugin
    -Werror
)

if(UNIX AND NOT MACOS)
    target_sources(hidplugin PUBLIC
        linux/hidapi.cpp
        linux/hidlinuxjoystick.cpp linux/hidlinuxjoystick.h
    )

    target_include_directories(hidplugin PRIVATE
        linux
    )
endif()

if(MACOS)
    target_sources(hidplugin PUBLIC
        macx/hidapi.cpp
        macx/hidosxjoystick.cpp macx/hidosxjoystick.h
    )

    set_target_properties(hidplugin PROPERTIES
        MACOSX_BUNDLE FALSE
    )
    target_include_directories(hidplugin PRIVATE
        macx
    )

    target_link_libraries(hidplugin PRIVATE
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

if(WIN32)
    target_sources(hidplugin PUBLIC
        win32/hidapi.cpp
        win32/hidwindowsjoystick.cpp win32/hidwindowsjoystick.h
    )

    target_link_libraries(hidplugin PRIVATE
        hid
        setupapi
        winmm
    )
endif()

if(qmlui)
    target_compile_definitions(hidplugin PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(hidplugin
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(hidplugin
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS hidplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
