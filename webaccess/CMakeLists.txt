project(webaccess)

add_subdirectory(src)
add_subdirectory(res)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(test)
endif()
