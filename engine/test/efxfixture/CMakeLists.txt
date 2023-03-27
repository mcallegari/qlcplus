add_executable(efxfixture_test WIN32
    ../common/resource_paths.h
    ../mastertimer/mastertimer_stub.cpp ../mastertimer/mastertimer_stub.h
    efxfixture_test.cpp efxfixture_test.h
)
target_include_directories(efxfixture_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../mastertimer
)

target_link_libraries(efxfixture_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(efxfixture_test
    -Werror
)

if(qmlui)
    target_compile_definitions(efxfixture_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(efxfixture_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(efxfixture_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(efxfixture_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(CONFIG(coverage))
    target_link_libraries(efxfixture_test PRIVATE
        gcov
    )

    target_compile_options(efxfixture_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS efxfixture_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
