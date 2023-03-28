include(../../../variables.cmake)
include(../../../coverage.cmake)

add_executable(qlcfixturedefcache_test WIN32
    ../common/resource_paths.h
    qlcfixturedefcache_test.cpp qlcfixturedefcache_test.h
)
target_include_directories(qlcfixturedefcache_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcfixturedefcache_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

if(IS_TRAVIS___contains___ON)
    target_compile_definitions(qlcfixturedefcache_test PRIVATE
        SKIP_TEST
    )
endif()

# if(count(IS_BUILDBOT, 1))
if(IS_BUILDBOT)
    target_compile_definitions(qlcfixturedefcache_test PRIVATE
        SKIP_TEST
    )
endif()

install(TARGETS qlcfixturedefcache_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
