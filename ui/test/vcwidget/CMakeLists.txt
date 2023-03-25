qt_add_executable(vcwidget_test WIN32 MACOSX_BUNDLE
    stubwidget.cpp stubwidget.h
    vcwidget_test.cpp vcwidget_test.h
)
target_include_directories(vcwidget_test PRIVATE
    ../../../engine/src
    ../../../plugins/interfaces
    ../../src
    ../../src/virtualconsole
)

target_link_libraries(vcwidget_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    Qt::Widgets
    qlcplusengine
    qlcplusui
)

target_compile_options(vcwidget_test
    -Werror
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(vcwidget_test PRIVATE
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(vcwidget_test PRIVATE
        Qt::Script
    )
endif()

if(qmlui)
    target_compile_definitions(vcwidget_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(vcwidget_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(vcwidget_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(vcwidget_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS vcwidget_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
