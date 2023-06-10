project(platforms)

if(UNIX AND NOT MACOS)
    add_subdirectory(linux)
endif()
if(WIN32)
    add_subdirectory(windows)
endif()
