include(../../variables.cmake)

add_library(hidplugin)
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
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(UNIX AND NOT MACOS)
    target_sources(hidplugin PRIVATE
        linux/hidapi.cpp
        linux/hidlinuxjoystick.cpp linux/hidlinuxjoystick.h
    )

    target_include_directories(hidplugin PRIVATE
        linux
    )
endif()

if(MACOS)
    target_sources(hidplugin PRIVATE
        macx/hidapi.cpp
        macx/hidosxjoystick.cpp macx/hidosxjoystick.h
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
    target_sources(hidplugin PRIVATE
        win32/hidapi.cpp
        win32/hidwindowsjoystick.cpp win32/hidwindowsjoystick.h
    )

    target_link_libraries(hidplugin PRIVATE
        hid
        setupapi
        winmm
    )
endif()

install(TARGETS hidplugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
