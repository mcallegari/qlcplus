project(engine VERSION 1.0.0)

add_subdirectory(audio)
add_subdirectory(src)
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(test)
endif()
