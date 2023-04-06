include(../../../variables.cmake)
include(../../../coverage.cmake)

add_executable(cue_test WIN32
    cue_test.cpp cue_test.h
)
target_include_directories(cue_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(cue_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

install(TARGETS cue_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
