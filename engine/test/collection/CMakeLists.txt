add_executable(collection_test WIN32
    ../mastertimer/mastertimer_stub.cpp ../mastertimer/mastertimer_stub.h
    collection_test.cpp collection_test.h
)
target_include_directories(collection_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../mastertimer
)

target_link_libraries(collection_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(collection_test
    -Werror
)

if(qmlui)
    target_compile_definitions(collection_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(collection_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(collection_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(collection_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(CONFIG(coverage))
    target_link_libraries(collection_test PRIVATE
        gcov
    )

    target_compile_options(collection_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS collection_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
