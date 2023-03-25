qt_add_executable(qlcplus-launcher WIN32 MACOSX_BUNDLE
    launcher.cpp launcher.h
    main.cpp
)
target_include_directories(qlcplus-launcher PRIVATE
    ../engine/src
    ../plugins/interfaces
)

target_link_libraries(qlcplus-launcher PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

target_compile_options(qlcplus-launcher
    -Werror
)


# Resources:
set_source_files_properties("../resources/icons/png/qlcplus-fixtureeditor.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus-fixtureeditor.png"
)
set_source_files_properties("../resources/icons/png/qlcplus.png"
    PROPERTIES QT_RESOURCE_ALIAS "qlcplus.png"
)
set(launcher_resource_files
    "../resources/icons/png/qlcplus-fixtureeditor.png"
    "../resources/icons/png/qlcplus.png"
)

qt_add_resources(qlcplus-launcher "launcher"
    PREFIX
        "/"
    FILES
        ${launcher_resource_files}
)

if(qmlui)
    target_compile_definitions(qlcplus-launcher PRIVATE
        QMLUI
    )
endif()

if((NOT MACOS AND NOT IOS) AND (system(g++--version OR grep-e"4.6.[0-9]")))
    target_compile_options(qlcplus-launcher
        -Wno-error=strict-overflow
    )
endif()

if((NOT MACOS AND NOT IOS) AND (NOT (system(g++--version OR grep-e"4.6.[0-9]"))))
    target_compile_options(qlcplus-launcher
        -Wno-unused-local-typedefs
    )
endif()

if(MACOS)
    set_target_properties(qlcplus-launcher PROPERTIES
        MACOSX_BUNDLE FALSE
    )
endif()

install(TARGETS qlcplus-launcher
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
