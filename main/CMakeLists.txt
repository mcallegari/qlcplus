qt_add_executable(qlcplus WIN32 MACOSX_BUNDLE
    main.cpp
)
target_include_directories(qlcplus PRIVATE
    ../engine/src
    ../ui/src
    ../ui/src/virtualconsole
    ../webaccess/src
)

target_link_libraries(qlcplus PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    qlcplusengine
    qlcplusui
    qlcpluswebaccess
)

target_compile_options(qlcplus
    -Werror
)

if(WIN32)
    target_sources(qlcplus PUBLIC
        ../ui/src/debugbox.cpp ../ui/src/debugbox.h ../ui/src/debugbox.ui
    )
endif()

if(MACOS)
    target_sources(qlcplus PUBLIC
        ../ui/src/debugbox.cpp ../ui/src/debugbox.h ../ui/src/debugbox.ui
    )

    set_target_properties(qlcplus PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(qlcplus PRIVATE
        Qt::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(qlcplus PRIVATE
        Qt::Script
    )
endif()

if(qmlui)
    target_compile_definitions(qlcplus PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(qlcplus
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(qlcplus
        -Wno-unused-local-typedefs
    )
endif()

install(TARGETS qlcplus
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
