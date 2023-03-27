add_executable(bus_test WIN32
    bus_test.cpp bus_test.h
)
target_include_directories(bus_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(bus_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(bus_test
    -Werror
)

if(qmlui)
    target_compile_definitions(bus_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(bus_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(bus_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(bus_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(coverage)
    target_link_libraries(bus_test PRIVATE
        gcov
    )

    target_compile_options(bus_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS bus_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
