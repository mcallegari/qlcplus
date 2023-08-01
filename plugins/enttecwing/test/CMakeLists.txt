
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
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Network
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets
    enttecwing
)

if(WIN32)
    target_compile_definitions(enttecwing_test PRIVATE
        QLC_EXPORT
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
