add_executable(rgbalgorithm_test WIN32
    ../common/resource_paths.h
    rgbalgorithm_test.cpp rgbalgorithm_test.h
)
target_include_directories(rgbalgorithm_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../mastertimer
)

target_link_libraries(rgbalgorithm_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    qlcplusengine
)

target_compile_options(rgbalgorithm_test
    -Werror
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(rgbalgorithm_test PRIVATE
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(rgbalgorithm_test PRIVATE
        Qt::Script
    )
endif()

if(qmlui)
    target_compile_definitions(rgbalgorithm_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(rgbalgorithm_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(rgbalgorithm_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(rgbalgorithm_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(coverage)
    target_link_libraries(rgbalgorithm_test PRIVATE
        gcov
    )

    target_compile_options(rgbalgorithm_test
        -fprofile-arcs
        -ftest-coverage
    )
endif()

install(TARGETS rgbalgorithm_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
