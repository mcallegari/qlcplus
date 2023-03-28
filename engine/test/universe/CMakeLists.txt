include(../../../variables.cmake)
include(../../../coverage.cmake)

add_executable(universe_test WIN32
    universe_test.cpp universe_test.h
)
target_include_directories(universe_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(universe_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

install(TARGETS universe_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
