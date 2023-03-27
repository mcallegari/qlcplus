add_executable(test WIN32 MACOSX_BUNDLE
    hpmtest.cpp hpmtest.h
    main.cpp
)
target_include_directories(test PRIVATE
    ../src
)

target_link_libraries(test PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    hotplugmonitor
)

target_compile_options(test
    -Werror
)

if(qmlui)
    target_compile_definitions(test PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(test
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(test
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(test PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(CONFIG(iokit))
    target_link_libraries(test PRIVATE
        "-framework CoreFoundation"
        "-framework IOKit"
    )
endif()

install(TARGETS test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
