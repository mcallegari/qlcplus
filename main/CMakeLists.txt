include(../variables.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(qlcplus WIN32 MACOSX_BUNDLE
    main.cpp
)
target_include_directories(qlcplus PRIVATE
    ../engine/src
    ../ui/src
    ../ui/src/virtualconsole
    ../webaccess/src
)

target_link_libraries(qlcplus PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    qlcplusengine
    qlcplusui
    qlcpluswebaccess
)

if(WIN32)
    target_sources(qlcplus PRIVATE
        ../ui/src/debugbox.cpp ../ui/src/debugbox.h ../ui/src/debugbox.ui
    )
endif()

if(MACOS)
    target_sources(qlcplus PRIVATE
        ../ui/src/debugbox.cpp ../ui/src/debugbox.h ../ui/src/debugbox.ui
    )
endif()

if(qmlui OR (QT_VERSION_MAJOR GREATER 5))
    target_link_libraries(qlcplus PRIVATE
        Qt${QT_MAJOR_VERSION}::Qml
    )
endif()

if(NOT (qmlui OR (QT_VERSION_MAJOR GREATER 5)))
    target_link_libraries(qlcplus PRIVATE
        Qt${QT_MAJOR_VERSION}::Script
    )
endif()

install(TARGETS qlcplus
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
