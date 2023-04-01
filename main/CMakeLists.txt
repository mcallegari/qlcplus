include(../variables.cmake)

set(module_name "qlcplus")

add_executable(${module_name} WIN32 MACOSX_BUNDLE
    main.cpp
)

target_include_directories(${module_name} PRIVATE
    ../engine/src
    ../ui/src
    ../ui/src/virtualconsole
    ../webaccess/src
)

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
    qlcplusui
    qlcpluswebaccess
)


qt5_add_resources(qlcplusui_resource_files ${CMAKE_SOURCE_DIR}/ui/src/qlcui.qrc)
target_sources(${module_name} PRIVATE
    ${qlcplusui_resource_files}
)

if(WIN32 OR MACOS)
    target_sources(${module_name} PRIVATE
        ../ui/src/debugbox.cpp ../ui/src/debugbox.h ../ui/src/debugbox.ui
    )
endif()

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Qml
    )
else()
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

install(TARGETS ${module_name}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
