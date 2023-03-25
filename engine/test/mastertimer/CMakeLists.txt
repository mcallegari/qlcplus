qt_add_executable(mastertimer_test WIN32
    ../../../plugins/interfaces/qlcioplugin.cpp
    ../common/resource_paths.h
    ../function/function_stub.cpp ../function/function_stub.h
    dmxsource_stub.cpp dmxsource_stub.h
    mastertimer_test.cpp mastertimer_test.h
)
target_include_directories(mastertimer_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../function
)

target_link_libraries(mastertimer_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(mastertimer_test
    -Werror
)

if(IS_TRAVIS___contains___ON)
    target_compile_definitions(mastertimer_test PRIVATE
        SKIP_TEST
    )
endif()

if(count(IS_BUILDBOT, 1))
    target_compile_definitions(mastertimer_test PRIVATE
        SKIP_TEST
    )
endif()

if(qmlui)
    target_compile_definitions(mastertimer_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(mastertimer_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(mastertimer_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(mastertimer_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(CONFIG(coverage))
    target_link_libraries(mastertimer_test PRIVATE
        gcov
    )

    target_compile_options(mastertimer_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS mastertimer_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
