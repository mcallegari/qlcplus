project(artnet)

add_subdirectory(src)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(test)
endif()
