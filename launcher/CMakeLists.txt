set(qlcplus-launcher_src
    launcher.cpp 
    main.cpp
)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/engine
    ${CMAKE_SOURCE_DIR}/engine/src
)

add_executable (qlcplus-launcher ${qlcplus-launcher_src})
target_link_libraries(qlcplus-launcher 
    Qt5::Widgets
)
