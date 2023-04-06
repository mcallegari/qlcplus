include(../../../variables.cmake)
include(../../../coverage.cmake)

add_executable(outputpatch_test WIN32
    outputpatch_test.cpp outputpatch_test.h
)
target_include_directories(outputpatch_test PRIVATE
    ../../../plugins/interfaces
    ../../src
    ../iopluginstub
)

target_link_libraries(outputpatch_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

install(TARGETS outputpatch_test
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
