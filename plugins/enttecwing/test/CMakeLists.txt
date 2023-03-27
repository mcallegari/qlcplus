add_executable(enttecwing_test WIN32 MACOSX_BUNDLE
    main.cpp
    playbackwing_test.cpp playbackwing_test.h
    programwing_test.cpp programwing_test.h
    shortcutwing_test.cpp shortcutwing_test.h
    wing_test.cpp wing_test.h
)
target_include_directories(enttecwing_test PRIVATE
    ../../interfaces
    ../src
)

target_link_libraries(enttecwing_test PRIVATE
    # Remove: L../src
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Test
    Qt::Widgets
    enttecwing
)

target_compile_options(enttecwing_test
    -Werror
)

if(qmlui)
    target_compile_definitions(enttecwing_test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(enttecwing_test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(enttecwing_test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(enttecwing_test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS enttecwing_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
