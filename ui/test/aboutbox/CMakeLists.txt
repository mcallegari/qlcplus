add_executable(aboutbox_test WIN32 MACOSX_BUNDLE
    aboutbox_test.cpp aboutbox_test.h
)
target_include_directories(aboutbox_test PRIVATE
    ../../../engine/src
    ../../src
)

target_link_libraries(aboutbox_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    Qt::Widgets
    qlcplusengine
    qlcplusui
)

target_compile_options(aboutbox_test
    -Werror
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(aboutbox_test PRIVATE
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(aboutbox_test PRIVATE
        Qt::Script
    )
endif()

if(qmlui)
    target_compile_definitions(aboutbox_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(aboutbox_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(aboutbox_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(aboutbox_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS aboutbox_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
