project(peperoni)

if(WIN32)
    add_subdirectory(win32)
endif()
if(UNIX)
    add_subdirectory(unix)
endif()
