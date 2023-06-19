add_executable(qlcpoint_test WIN32
    qlcpoint_test.cpp qlcpoint_test.h
)
target_include_directories(qlcpoint_test PRIVATE
    ../../../plugins/interfaces
    ../../src
)

target_link_libraries(qlcpoint_test PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    qlcplusengine
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
