qt_add_library(hotplugmonitor
    hotplugmonitor.cpp hotplugmonitor.h
)
target_link_libraries(hotplugmonitor PUBLIC
    Qt::Core
    Qt::Gui
)

target_compile_options(hotplugmonitor
    -Werror
)

if(CONFIG(udev))
    target_sources(hotplugmonitor PUBLIC
        hpmprivate-udev.cpp hpmprivate-udev.h
    )
endif()

if(CONFIG(iokit))
    target_sources(hotplugmonitor PUBLIC
        hpmprivate-iokit.cpp hpmprivate-iokit.h
    )

    target_link_libraries(hotplugmonitor PUBLIC
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

if(CONFIG(win32))
    target_sources(hotplugmonitor PUBLIC
        hpmprivate-win32.cpp hpmprivate-win32.h
    )
endif()

if(qmlui)
    target_compile_definitions(hotplugmonitor PUBLIC
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(hotplugmonitor
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(hotplugmonitor
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(hotplugmonitor PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS hotplugmonitor
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
