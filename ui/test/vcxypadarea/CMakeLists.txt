add_executable(vcxypadarea_test WIN32 MACOSX_BUNDLE
    vcxypadarea_test.cpp vcxypadarea_test.h
)
target_include_directories(vcxypadarea_test PRIVATE
    ../../../engine/src
    ../../../plugins/interfaces
    ../../src
    ../../src/virtualconsole
)

target_link_libraries(vcxypadarea_test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Test
    Qt::Widgets
    qlcplusengine
    qlcplusui
)

target_compile_options(vcxypadarea_test
    -Werror
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(vcxypadarea_test PRIVATE
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(vcxypadarea_test PRIVATE
        Qt::Script
    )
endif()

if(qmlui)
    target_compile_definitions(vcxypadarea_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(vcxypadarea_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(vcxypadarea_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(vcxypadarea_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS vcxypadarea_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
