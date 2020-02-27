set(hotplugmonitor_src
    src/hotplugmonitor.cpp
)

set(hotplugmonitor_link
    Qt5::Core
)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(hotplugmonitor_src
        ${hotplugmonitor_src}
        src/hpmprivate-win32.cpp
    )
    set(hotplugmonitor_link
        ${hotplugmonitor_link}
    )
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(IOKit REQUIRED)
    set(hotplugmonitor_src
        ${hotplugmonitor_src}
        src/hpmprivate-iokit.cpp
    )
    set(hotplugmonitor_link
        ${hotplugmonitor_link}
        ${IOKit_LIBRARY}
    )
ELSE()
    find_package(udev REQUIRED)
    set(hotplugmonitor_src
        ${hotplugmonitor_src}
        src/hpmprivate-udev.cpp
    )
    set(hotplugmonitor_link
        ${hotplugmonitor_link}
        ${UDEV_LIBRARIES}
    )
ENDIF()

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${UDEV_INCLUDE_DIRS} OPTIONAL
    ${IOKit_INCLUDE_DIR} OPTIONAL
)

add_library(hotplugmonitor STATIC ${hotplugmonitor_src})
target_link_libraries(hotplugmonitor ${hotplugmonitor_link})

#runnig tests
add_executable(hpmtest 
    test/hpmtest.cpp
    test/main.cpp
)
target_link_libraries(hpmtest hotplugmonitor Qt5::Widgets)

#add_test(hpmtest hpmtest)
