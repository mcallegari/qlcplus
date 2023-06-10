project(hotplugmonitor)

if(NOT ANDROID AND NOT IOS)
    add_subdirectory(src)
    add_subdirectory(test)
endif()
