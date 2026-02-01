add_library(hotplugmonitor
    STATIC
    hotplugmonitor.cpp hotplugmonitor.h
)
set_property(TARGET hotplugmonitor PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(hotplugmonitor PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
)

if(udev)
    target_sources(hotplugmonitor PRIVATE
        hpmprivate-udev.cpp hpmprivate-udev.h
    )
    target_link_libraries(hotplugmonitor PRIVATE
        udev
    )
endif()

if(iokit)
    target_sources(hotplugmonitor PRIVATE
        hpmprivate-iokit.cpp hpmprivate-iokit.h
    )

    target_link_libraries(hotplugmonitor PUBLIC
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

if(WIN32)
    target_sources(hotplugmonitor PRIVATE
        hpmprivate-win32.cpp hpmprivate-win32.h
    )
endif()
