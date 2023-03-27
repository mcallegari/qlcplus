add_executable(cue_test WIN32
    cue_test.cpp cue_test.h
)
target_include_directories(cue_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(cue_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(cue_test
    -Werror
)

if(qmlui)
    target_compile_definitions(cue_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(cue_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(cue_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(cue_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(coverage)
    target_link_libraries(cue_test PRIVATE
        gcov
    )

    target_compile_options(cue_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS cue_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
