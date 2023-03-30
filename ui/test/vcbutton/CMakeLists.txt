include(../../../variables.cmake)

if(MSVC)
    add_executable(vcbutton_test WIN32 MACOSX_BUNDLE
        vcbutton_test.cpp vcbutton_test.h
        ../../src/virtualconsole/vcbutton.h
    )
else()
    add_executable(vcbutton_test WIN32 MACOSX_BUNDLE
        vcbutton_test.cpp vcbutton_test.h
    )
endif()

target_include_directories(vcbutton_test PRIVATE
    ../../../engine/src
    ../../../plugins/interfaces
    ../../src
    ../../src/virtualconsole
)

target_link_libraries(vcbutton_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
    qlcplusui
)

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(vcbutton_test PRIVATE
        Qt${QT_MAJOR_VERSION}::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(vcbutton_test PRIVATE
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

install(TARGETS vcbutton_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
