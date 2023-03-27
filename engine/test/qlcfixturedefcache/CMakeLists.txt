add_executable(qlcfixturedefcache_test WIN32
    ../common/resource_paths.h
    qlcfixturedefcache_test.cpp qlcfixturedefcache_test.h
)
target_include_directories(qlcfixturedefcache_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcfixturedefcache_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(qlcfixturedefcache_test
    -Werror
)

if(IS_TRAVIS___contains___ON)
    target_compile_definitions(qlcfixturedefcache_test PRIVATE
        SKIP_TEST
    )
endif()

if(count(IS_BUILDBOT, 1))
    target_compile_definitions(qlcfixturedefcache_test PRIVATE
        SKIP_TEST
    )
endif()

if(qmlui)
    target_compile_definitions(qlcfixturedefcache_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(qlcfixturedefcache_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(qlcfixturedefcache_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(qlcfixturedefcache_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(CONFIG(coverage))
    target_link_libraries(qlcfixturedefcache_test PRIVATE
        gcov
    )

    target_compile_options(qlcfixturedefcache_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS qlcfixturedefcache_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
