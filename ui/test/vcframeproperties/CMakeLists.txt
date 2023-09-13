include(../../src/include_ui.cmake)

set(module_name "vcframeproperties_test")

add_executable(${module_name} WIN32 MACOSX_BUNDLE
    ${module_name}.cpp ${module_name}.h
)

target_include_directories(${module_name} PRIVATE
    ../../../engine/src
    ../../../plugins/interfaces
    ../../src
    ../../src/virtualconsole
)

include_ui_header(${module_name})

target_link_libraries(${module_name} PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
    qlcplusui
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(${module_name} PRIVATE
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
