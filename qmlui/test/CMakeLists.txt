# Reuse the frontend sources so the regression exercises the actual backend.
get_target_property(frontend_sources qlcplus5 SOURCES)
get_target_property(frontend_includes qlcplus5 INCLUDE_DIRECTORIES)
get_target_property(frontend_definitions qlcplus5 COMPILE_DEFINITIONS)
get_target_property(frontend_libraries qlcplus5 LINK_LIBRARIES)
set(test_sources sceneeditor_test.cpp)
foreach(source IN LISTS frontend_sources)
    if(source MATCHES "(^|/)main\\.cpp$" OR source MATCHES "\\.(qm|rc)$"
       OR source MATCHES "(^|/)qrc_[^/]*\\.cpp$")
        continue()
    endif()
    if(NOT IS_ABSOLUTE "${source}")
        set(source "${CMAKE_CURRENT_SOURCE_DIR}/../${source}")
    endif()
    list(APPEND test_sources "${source}")
endforeach()
set(test_includes "${CMAKE_CURRENT_SOURCE_DIR}/..")
foreach(directory IN LISTS frontend_includes)
    if(NOT IS_ABSOLUTE "${directory}")
        set(directory "${CMAKE_CURRENT_SOURCE_DIR}/../${directory}")
    endif()
    list(APPEND test_includes "${directory}")
endforeach()
qt_add_resources(test_resources ../qmlui.qrc ../../resources/icons/svg/svgicons.qrc
    ../../resources/fonts/fonts.qrc)
add_executable(sceneeditor_test ${test_sources} ${test_resources})
target_include_directories(sceneeditor_test PRIVATE ${test_includes})
target_compile_definitions(sceneeditor_test PRIVATE ${frontend_definitions})
target_link_libraries(sceneeditor_test PRIVATE ${frontend_libraries} Qt${QT_MAJOR_VERSION}::Test)

set(test_path "PATH=$ENV{PATH}")
if(WIN32)
    set(test_path "PATH=$<TARGET_FILE_DIR:qlcplusengine>;$<TARGET_FILE_DIR:hotplugmonitor>;$<TARGET_FILE_DIR:Qt${QT_MAJOR_VERSION}::Core>;$ENV{PATH}")
endif()
add_custom_target(check-sceneeditor
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen "${test_path}"
        $<TARGET_FILE:sceneeditor_test> -o sceneeditor-test-results.txt,txt
    COMMAND ${CMAKE_COMMAND} -E cat sceneeditor-test-results.txt
    DEPENDS sceneeditor_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)
