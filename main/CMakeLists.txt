set(qlcplus_src 
    main.cpp
)

include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/src
    ${CMAKE_SOURCE_DIR}/ui/src/
    ${CMAKE_SOURCE_DIR}/ui/src/virtualconsole
    ${CMAKE_SOURCE_DIR}/webaccess/src/
)

add_executable(qlcplus ${qlcplus_src})

target_link_libraries(qlcplus
    Qt5::Widgets
    qlcplusengine
    qlcplusui
    qlcpluswebaccess
)
